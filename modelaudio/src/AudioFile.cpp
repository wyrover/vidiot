#include "AudioFile.h"

#include "AudioChunk.h"
#include "AudioCompositionParameters.h"
#include "Convert.h"
#include "UtilInitAvcodec.h"
#include "UtilLog.h"

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
    ,   audioCombineBuffer(0)
{
    VAR_DEBUG(*this);
}

AudioFile::AudioFile(wxFileName path)
    :	File(path,sMaxBufferSize)
    ,   mResampleContext(0)
    ,   mDecodingAudio(false)
    ,   audioDecodeBuffer(0)
    ,   audioResampleBuffer(0)
    ,   audioCombineBuffer(0)
{
    VAR_DEBUG(*this);
}

AudioFile::AudioFile(const AudioFile& other)
    :   File(other)
    ,   mResampleContext(0)
    ,   mDecodingAudio(false)
    ,   audioDecodeBuffer(0)
    ,   audioResampleBuffer(0)
    ,   audioCombineBuffer(0)
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
    delete[] audioCombineBuffer;

    audioDecodeBuffer = 0;
    audioResampleBuffer = 0;
    audioCombineBuffer = 0;
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
    delete[] audioCombineBuffer;

    audioDecodeBuffer = 0;
    audioResampleBuffer = 0;
    audioCombineBuffer = 0;

    File::clean();
}

//////////////////////////////////////////////////////////////////////////
// IAUDIO
//////////////////////////////////////////////////////////////////////////

AudioChunkPtr AudioFile::getNextAudio(const AudioCompositionParameters& parameters)
{
    startDecodingAudio(parameters);

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
    int targetSizeInBytes = 0;
    ASSERT_MORE_THAN_ZERO(sourceSize);

    AVCodecContext* codec = getCodec();

    while (sourceSize > 0)
    {
        int decodeSizeInBytes = sAudioBufferSizeInBytes - targetSizeInBytes; // Needed for avcodec_decode_audio2(): Initially this must be set to the maximum to be decoded bytes
        AVPacket packet;
        packet.data = sourceData;
        packet.size = sourceSize;
        int usedSourceBytes = avcodec_decode_audio3(codec, targetData, &decodeSizeInBytes, &packet);
        ASSERT_MORE_THAN_EQUALS_ZERO(usedSourceBytes);

        if (decodeSizeInBytes <= 0)
        {
            // if error, skip frame
            LOG_WARNING << "Frame skipped";
            sourceSize = 0;
            break;
        }

        sourceData += usedSourceBytes;
        sourceSize -= usedSourceBytes;

        ASSERT_ZERO(decodeSizeInBytes % 2)(decodeSizeInBytes);
        targetSizeInBytes += decodeSizeInBytes;
        targetData += decodeSizeInBytes / AudioChunk::sBytesPerSample; // av_get_bytes_per_sample(codec->sample_fmt) ??
    }
    ASSERT_ZERO(sourceSize);

    //////////////////////////////////////////////////////////////////////////
    // RESAMPLING

    int nSamples = targetSizeInBytes / AudioChunk::sBytesPerSample; // A sample is the data for one speaker

    typedef boost::rational<int> rational;
    auto convertOutputSampleCountToInputSampleCount = [parameters,codec](int output) -> int
    {
        return
            Convert::toInt(rational(output) *
            rational(codec->channels) / rational(parameters.getNrChannels()) *
            rational(codec->sample_rate) / rational(parameters.getSampleRate()));
    };

    if (mResampleContext != 0)
    {
        int nRemainingInputSamples = nSamples;
        nSamples = 0;

        // Note: Sometimes audio_resample does not resample the entire buffer in one pass. Here the extra required passes are done, and the data is copied into audioCombineBuffer.
        // todo all this memcpy-ing canbe removed by pre-allocating the returned audiochunk and then storing the data in there?

        sample* input = audioDecodeBuffer;
        sample* output = audioCombineBuffer;

        while (nRemainingInputSamples > 0)
        {
            int nInputFrames = nRemainingInputSamples / codec->channels;
            int nOutputFrames = audio_resample(mResampleContext, audioResampleBuffer, input, nInputFrames);
            int nNewOutputSamples = nOutputFrames * parameters.getNrChannels();
            int nUsedInputSamples = convertOutputSampleCountToInputSampleCount(nNewOutputSamples);
            VAR_DEBUG(nInputFrames)(nOutputFrames)(nNewOutputSamples)(nUsedInputSamples);
            if (nUsedInputSamples == 0)
            {
                break;
            }

            memcpy(output,audioResampleBuffer,nNewOutputSamples * AudioChunk::sBytesPerSample);

            nSamples += nNewOutputSamples;
            nRemainingInputSamples -= nUsedInputSamples;
            input += nUsedInputSamples;
            output += nNewOutputSamples;
            // NOT: ASSERT_MORE_THAN_EQUALS_ZERO(nRemainingInputSamples)(nUsedInputSamples);
            // RATIONALE: Sometimes audio_resample returns a slightly higher number of output frames then to be expected
            //            by the given amount of input frames (maybe caused by audio_resample mechanism 'caching' some of
            //            them in a previous call?).
        }

        targetData = audioCombineBuffer;

        VAR_DEBUG(nSamples);
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
        NIY(_("Not supported: Audio data without pts info"));
    }

    // todo make generic method for determining audio pts values. For transition this will result in wrong numbers
    pts += static_cast<double>(nSamples) / static_cast<double>(parameters.getSampleRate()); // nAudioChannels already done before resampling

    AudioChunkPtr audioChunk = boost::make_shared<AudioChunk>(targetData, parameters.getNrChannels(), nSamples, Convert::doubleToInt(pts));
    VAR_AUDIO(this)(audioChunk);
    return audioChunk;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void AudioFile::startDecodingAudio(const AudioCompositionParameters& parameters)
    //int audioRate, int nAudioChannels)
{
    if (mDecodingAudio) return;

    // Allocated upon first use. See also the remark in the header file
    // on GCC in combination with make_shared.
    if (!audioDecodeBuffer)
    {
        audioDecodeBuffer   = new sample[sAudioBufferSize];
        audioResampleBuffer = new sample[sAudioBufferSize];
        audioCombineBuffer  = new sample[sAudioBufferSize];
    }

    startReadingPackets(); // Also causes the file to be opened resulting in initialized avcodec members for File.
    mDecodingAudio = true;

    boost::mutex::scoped_lock lock(sMutexAvcodec);

    AVCodecContext* codec = getCodec();

    AVCodec* audioCodec = avcodec_find_decoder(codec->codec_id);
    ASSERT_NONZERO(audioCodec);

    int result = avcodec_open(codec, audioCodec);
    ASSERT_MORE_THAN_EQUALS_ZERO(result);

    if (codec->sample_fmt != AV_SAMPLE_FMT_S16)
    {
        switch (codec->sample_fmt)
        {
        case AV_SAMPLE_FMT_U8:  NIY("Unsigned 8 bits audio is not yet supported!"); break;
        case AV_SAMPLE_FMT_S16: NIY("Signed 16 bits audio is not yet supported!");  break;
        case AV_SAMPLE_FMT_S32: NIY("Signed 32 bits audio is not yet supported!");  break;
        case AV_SAMPLE_FMT_FLT: NIY("Floating point audio is not yet supported!");  break;
        case AV_SAMPLE_FMT_DBL: NIY("Double type audio is not yet supported!");     break;
        default:                NIY("Unsupported audio type!");                     break;
        }
    }
    ASSERT_EQUALS(codec->sample_fmt,AV_SAMPLE_FMT_S16);

    if ((parameters.getNrChannels() != codec->channels) || (parameters.getSampleRate() != codec->sample_rate))
    {
        VAR_INFO(parameters.getNrChannels())(codec->channels)(parameters.getSampleRate())(codec->sample_rate);
        static const int taps = 16;
        mResampleContext =
            av_audio_resample_init(
                parameters.getNrChannels(), codec->channels,
                parameters.getSampleRate(), codec->sample_rate,
                AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_S16,// 0,0,0,0);
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

bool AudioFile::useStream(AVMediaType type) const
{
    return (type == AVMEDIA_TYPE_AUDIO);
}

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