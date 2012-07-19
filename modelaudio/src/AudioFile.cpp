#include "AudioFile.h"

// Include at top, to exclude the intmax macros and use the boost versions
#undef INTMAX_C
#undef UINTMAX_C
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
};

#include "UtilLog.h"
#include "UtilInitAvcodec.h"
#include "AudioChunk.h"
#include "Node.h"

namespace model
{
static const int sMicroSecondsPerSeconds = 1000 * 1000;
static const int sMaxBufferSize = 100;

static const int sAudioBufferSize = AVCODEC_MAX_AUDIO_FRAME_SIZE;
static const int sAudioBufferSizeInBytes = sAudioBufferSize * 2;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AudioFile::AudioFile()
    :	File()
    ,   mResampleContext(0)
    ,   mDecodingAudio(false)
    ,   audioDecodeBuffer(0)
    ,   audioResampleBuffer(0)
{
    VAR_DEBUG(*this);
}

AudioFile::AudioFile(wxFileName path)
    :	File(path,sMaxBufferSize)
    ,   mResampleContext(0)
    ,   mDecodingAudio(false)
    ,   audioDecodeBuffer(0)
    ,   audioResampleBuffer(0)
{
    VAR_DEBUG(*this);
}

AudioFile::AudioFile(const AudioFile& other)
    :   File(other)
    ,   mResampleContext(0)
    ,   mDecodingAudio(false)
    ,   audioDecodeBuffer(0)
    ,   audioResampleBuffer(0)
{
    VAR_DEBUG(*this);
}

AudioFile* AudioFile::clone() const
{
    return new AudioFile(static_cast<const AudioFile&>(*this));
}

AudioFile::~AudioFile()
{
    VAR_DEBUG(this);

    stopDecodingAudio();

    delete[] audioDecodeBuffer;
    delete[] audioResampleBuffer;
    audioDecodeBuffer = 0;
    audioResampleBuffer = 0;
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

void AudioFile::clean()
{
    VAR_DEBUG(this);

    stopDecodingAudio();

    delete[] audioDecodeBuffer;
    delete[] audioResampleBuffer;
    audioDecodeBuffer = 0;
    audioResampleBuffer = 0;

    File::clean();
}

//////////////////////////////////////////////////////////////////////////
// IAUDIO
//////////////////////////////////////////////////////////////////////////

AudioChunkPtr AudioFile::getNextAudio(int audioRate, int nAudioChannels)
{
    startDecodingAudio(audioRate,nAudioChannels);

    PacketPtr audioPacket = getNextPacket();
    if (!audioPacket)
    {
        // End of file reached. Signal this with null ptr.
        return AudioChunkPtr();
    }

    //////////////////////////////////////////////////////////////////////////
    // DECODING

    // All sizes are in bytes below
    uint8_t* sourceData = audioPacket->getPacket()->data;
    int16_t* targetData = audioDecodeBuffer;
    int sourceSize = audioPacket->getPacket()->size;
    int targetSize = 0;
    ASSERT_MORE_THAN_ZERO(sourceSize);

    while (sourceSize > 0)
    {
        int decodeSize = sAudioBufferSizeInBytes - targetSize; // Needed for avcodec_decode_audio2(): Initially this must be set to the maximum to be decoded bytes
        AVPacket packet;
        packet.data = sourceData;
        packet.size = sourceSize;
        int usedSourceBytes = avcodec_decode_audio3(getCodec(), targetData, &decodeSize, &packet);
        ASSERT_MORE_THAN_EQUALS_ZERO(usedSourceBytes);

        if (decodeSize <= 0)
        {
            // if error, skip frame
            LOG_WARNING << "Frame skipped";
            sourceSize = 0;
            break;
        }

        sourceData += usedSourceBytes;
        sourceSize -= usedSourceBytes;

        ASSERT_ZERO(decodeSize % 2)(decodeSize);
        targetSize += decodeSize;
        targetData += decodeSize / AudioChunk::sBytesPerSample;
    }

    //////////////////////////////////////////////////////////////////////////
    // RESAMPLING

    int nSamples = targetSize / AudioChunk::sBytesPerSample; // A sample is the data for one speaker
    int nFrames = nSamples / AudioChunk::sSamplesPerStereoFrame;         // A frame  is the data for all speakers

    if (mResampleContext != 0)
    {
        nFrames = audio_resample(mResampleContext, audioResampleBuffer, audioDecodeBuffer, nSamples / getCodec()->channels);
        nSamples = nFrames * AudioChunk::sSamplesPerStereoFrame;

        // Use the resampled data
        targetData = audioResampleBuffer;
    }
    else
    {
        // Use the plain decoded data without resampling.
        targetData = audioDecodeBuffer;
    }
    ASSERT_MORE_THAN_ZERO(nSamples);

    //////////////////////////////////////////////////////////////////////////
    // PTS

    double pts = 0;
    if (audioPacket->getPacket()->pts != AV_NOPTS_VALUE)
    {
        pts = av_q2d(getCodec()->time_base) * audioPacket->getPacket()->pts;
    }
    else
    {
        // To be implemented locally: 'make up' pts.
        NIY;
    }

    pts += static_cast<double>(nSamples) / static_cast<double>(/*nAudioChannels * already done before resampling */audioRate);

    AudioChunkPtr audioChunk = boost::make_shared<AudioChunk>(targetData, nAudioChannels, nSamples, pts);
    VAR_AUDIO(this)(audioChunk);
    return audioChunk;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void AudioFile::startDecodingAudio(int audioRate, int nAudioChannels)
{
    if (mDecodingAudio) return;

    // Allocated upon first use. See also the remark in the header file
    // on GCC in combination with make_shared.
    if (!audioDecodeBuffer)
    {
        audioDecodeBuffer   = new sample[sAudioBufferSize];
        audioResampleBuffer = new sample[sAudioBufferSize];
    }

    startReadingPackets(); // Also causes the file to be opened resulting in initialized avcodec members for File.
    mDecodingAudio = true;

    boost::mutex::scoped_lock lock(sMutexAvcodec);

    AVCodec* audioCodec = avcodec_find_decoder(getCodec()->codec_id);
    ASSERT_NONZERO(audioCodec);

    int result = avcodec_open(getCodec(), audioCodec);
    ASSERT_MORE_THAN_EQUALS_ZERO(result);

    ASSERT_EQUALS(getCodec()->sample_fmt,AV_SAMPLE_FMT_S16); // TODO handle more nicely (present dialog 'not supported yet', or solve, add support for other sample formats, and test)
    //AV_SAMPLE_FMT_U8,          ///< unsigned 8 bits
    //AV_SAMPLE_FMT_S16,         ///< signed 16 bits
    //AV_SAMPLE_FMT_S32,         ///< signed 32 bits
    //AV_SAMPLE_FMT_FLT,         ///< float
    //AV_SAMPLE_FMT_DBL,         ///< double

    if ((nAudioChannels != getCodec()->channels) || (audioRate != getCodec()->sample_rate))
    {
        LOG_INFO << "Resampling initialized";
        static const int taps = 16;
        mResampleContext =
            av_audio_resample_init(
                nAudioChannels, getCodec()->channels,
                audioRate, getCodec()->sample_rate,
                AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_S16,
                taps, 10, 0, 0.8);
        ASSERT_NONZERO(mResampleContext);
    }

    VAR_DEBUG(this)(getCodec());
}

void AudioFile::stopDecodingAudio()
{
    VAR_DEBUG(this);
    if (mDecodingAudio)
    {
        boost::mutex::scoped_lock lock(sMutexAvcodec);

        if (mResampleContext != 0)
        {
            LOG_INFO << "Resampling ended";
            audio_resample_close(mResampleContext);
            mResampleContext = 0;
        }

        avcodec_close(getCodec());
    }
    mDecodingAudio = false;
}

//////////////////////////////////////////////////////////////////////////
// FROM FILE
//////////////////////////////////////////////////////////////////////////

void AudioFile::flush()
{
    if (mDecodingAudio)
    {
        avcodec_flush_buffers(getCodec());
    }
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const AudioFile& obj )
{
    os << static_cast<const File&>(obj) << '|' << obj.mDecodingAudio;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void AudioFile::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<File>(*this);
    ar & boost::serialization::base_object<IAudio>(*this);
}
template void AudioFile::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void AudioFile::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);
} //namespace