#include "AudioFile.h"

// Include at top, to exclude the intmax macros and use the boost versions
#undef INTMAX_C
#undef UINTMAX_C
extern "C" {
#include <avformat.h>
#include <avcodec.h>
};

#include <math.h>
#include <algorithm>
#include <boost/make_shared.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include "UtilLog.h"
#include "UtilLogAvcodec.h"
#include "AProjectViewNode.h"

namespace model
{

static const int sMicroSecondsPerSeconds = 1000 * 1000;
static const int sBytesPerSample = 2;
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
    mCodecType = AVMEDIA_TYPE_AUDIO;
    VAR_DEBUG(*this);
}

AudioFile::AudioFile(boost::filesystem::path path)
    :	File(path,sMaxBufferSize)
    ,   mResampleContext(0)
    ,   mDecodingAudio(false)
    ,   audioDecodeBuffer(0)
    ,   audioResampleBuffer(0)
{
    mCodecType = AVMEDIA_TYPE_AUDIO;
    VAR_DEBUG(*this);
    /** /todo asserts on sample sizes. Only 16 bits data supported (resampling/decoding?)  */
}

AudioFile::AudioFile(const AudioFile& other)
    :   File(other)
    ,   mResampleContext(0)
    ,   mDecodingAudio(false)
    ,   audioDecodeBuffer(0)
    ,   audioResampleBuffer(0)
{
    mCodecType = AVMEDIA_TYPE_AUDIO;
    VAR_DEBUG(*this);
}

AudioFile* AudioFile::clone()
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

void AudioFile::startDecodingAudio(int audioRate, int nAudioChannels)
{
    if (mDecodingAudio) return;

    // Allocated upon first use. See also the remark in the header file
    // on GCC in combination with make_shared.
    if (!audioDecodeBuffer)
    {
        audioDecodeBuffer = new boost::int16_t[sAudioBufferSize];
        audioResampleBuffer = new boost::int16_t[sAudioBufferSize];
    }

    startReadingPackets(); // Also causes the file to be opened resulting in initialized avcodec members for File.
    mDecodingAudio = true;

    boost::mutex::scoped_lock lock(sMutexAvcodec);

    mCodecContext = mStream->codec;

    AVCodec* audioCodec = avcodec_find_decoder(mCodecContext->codec_id);
    ASSERT(audioCodec != 0)(audioCodec);

    int result = avcodec_open(mCodecContext, audioCodec);
    ASSERT(result >= 0)(result);

    if ((nAudioChannels != mCodecContext->channels) || (audioRate != mCodecContext->sample_rate))
    {
        LOG_INFO << "Resampling initialized";
        static const int taps = 16;
        mResampleContext =
            av_audio_resample_init(
                nAudioChannels, mCodecContext->channels,
                audioRate, mCodecContext->sample_rate,
                SAMPLE_FMT_S16, SAMPLE_FMT_S16,
                taps, 10, 0, 0.8);
        ASSERT(mResampleContext != 0);/** /todo replace with gui message and abort */
    }

    VAR_DEBUG(this)(mCodecContext);
}

void AudioFile::stopDecodingAudio()
{
    VAR_DEBUG(this);
    if (mDecodingAudio)
    {
        boost::mutex::scoped_lock lock(sMutexAvcodec);
        avcodec_close(mCodecContext);
    }
    mDecodingAudio = false;
}

AudioChunkPtr AudioFile::getNextAudio(int audioRate, int nAudioChannels)
{
    // @todo if the end of file is reached, a subsequent getNextAudio will trigger a new 
    // (useless) sequence of startReadingPackets, bufferPacketsThread, "bufferPacketsThread: End of file."
    // (and this, over and over again....). Also for video?
    // Basically, in the Audio/Video-File class, buffering should not be restarted when the whole of the
    // file has already been buffered (not until a 'moveto', at least).
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
    ASSERT(sourceSize > 0)(sourceSize);

    while (sourceSize > 0)
    {
        int decodeSize = sAudioBufferSizeInBytes - targetSize; // Needed for avcodec_decode_audio2(): Initially this must be set to the maximum to be decoded bytes
        /** /todo replace with decode_audio3 */
        int usedSourceBytes = avcodec_decode_audio2(mCodecContext, targetData, &decodeSize, sourceData, sourceSize);
        ASSERT(usedSourceBytes >= 0)(usedSourceBytes);

        if (decodeSize <= 0)
        {
            // if error, skip frame
            LOG_WARNING << "Frame skipped";
            sourceSize = 0;
            break;
        }

        sourceData += usedSourceBytes;
        sourceSize -= usedSourceBytes;

        ASSERT(decodeSize % 2 == 0)(decodeSize);
        targetSize += decodeSize;
        targetData += decodeSize / AudioChunk::sBytesPerSample;
    }

    //////////////////////////////////////////////////////////////////////////
    // RESAMPLING

    const unsigned int sSamplesPerStereoFrame = 2;
    int nSamples = targetSize / AudioChunk::sBytesPerSample; // A sample is the data for one speaker
    int nFrames = nSamples / sSamplesPerStereoFrame;         // A frame  is the data for all speakers

    if (mResampleContext != 0)
    {
        nFrames = audio_resample(mResampleContext, audioResampleBuffer, audioDecodeBuffer, nSamples / mCodecContext->channels);
        nSamples = nFrames * sSamplesPerStereoFrame;

        // Use the resampled data
        targetData = audioResampleBuffer;
    }
    else
    {
        // Use the plain decoded data without resampling.
        targetData = audioDecodeBuffer;
    }
    ASSERT(nSamples > 0)(nSamples);

    //////////////////////////////////////////////////////////////////////////
    // PTS

    double pts = 0;
    if (audioPacket->getPacket()->pts != AV_NOPTS_VALUE)
    {
        pts = av_q2d(mCodecContext->time_base) * audioPacket->getPacket()->pts;
    }
    else
    {
        // To be implemented locally: 'make up' pts.
        NIY;
    }

    pts += static_cast<double>(nSamples) / static_cast<double>(/*nAudioChannels * already done before resampling */audioRate);

    AudioChunkPtr audioChunk = boost::make_shared<AudioChunk>(targetData, nAudioChannels, nSamples, pts);//boost::make_shared<AudioChunk>(audioDecodeBuffer, outputSize / 2, pts);
    VAR_AUDIO(this)(audioChunk);
    return audioChunk;
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
