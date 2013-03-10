#include "AudioFile.h"

#include "AudioChunk.h"
#include "AudioCompositionParameters.h"
#include "Convert.h"
#include "UtilInitAvcodec.h"
#include "UtilLog.h"
#include "UtilLogAvcodec.h"

namespace model
{
static const int sMicroSecondsPerSeconds = 1000 * 1000;
static const int sMaxBufferSize = 100;

static const int sAudioBufferSizeInBytes = AVCODEC_MAX_AUDIO_FRAME_SIZE;

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

AudioChunkPtr AudioFile::getNextAudio(const AudioCompositionParameters& parameters)
{
    startDecodingAudio(parameters);

    PacketPtr audioPacket = getNextPacket();
    if (!audioPacket)
    {
        // End of file reached. Signal this with null ptr.
        return AudioChunkPtr();
    }

    auto determinePts = [this,audioPacket,parameters] (int nSamples) -> pts
    {
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

        // Note: if the code below is ever included, beware that the pts value for audioChunk is set BEFORE knowing the actual amount of returned samples,
        //       particularly in case resampling is involved.
        // pts += static_cast<double>(nSamples) / static_cast<double>(parameters.getNrChannels()) / static_cast<double>(parameters.getSampleRate()); Use the original pts value, which indicates the beginning pts value
        return Convert::doubleToInt(pts);
    };

    //////////////////////////////////////////////////////////////////////////
    // DECODING

    // All sizes are in bytes below
    uint8_t* sourceData = audioPacket->getPacket()->data;
    int16_t* targetData = audioDecodeBuffer;
    int sourceSize = audioPacket->getPacket()->size;
    int targetSizeInBytes = 0;
    ASSERT_MORE_THAN_ZERO(sourceSize);

    AVCodecContext* codec = getCodec();

    int nBytesPerSample = av_get_bytes_per_sample(codec->sample_fmt);
    ASSERT_NONZERO(nBytesPerSample);

    while (sourceSize > 0)
    {
        int decodeSizeInBytes = sAudioBufferSizeInBytes - targetSizeInBytes; // Needed for avcodec_decode_audio2(): Initially this must be set to the maximum to be decoded bytes
        AVPacket packet;
        packet.data = sourceData;
        packet.size = sourceSize;
        int usedSourceBytes = avcodec_decode_audio3(codec, targetData, &decodeSizeInBytes, &packet); // todo use avcodec_decode_audio4
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

        ASSERT_ZERO(decodeSizeInBytes % nBytesPerSample)(decodeSizeInBytes);
        targetSizeInBytes += decodeSizeInBytes;
        targetData += decodeSizeInBytes / nBytesPerSample;
    }
    ASSERT_ZERO(sourceSize);

    //////////////////////////////////////////////////////////////////////////
    // RESAMPLING

    int nDecodedSamples = targetSizeInBytes / nBytesPerSample; // A sample is the data for one speaker
    ASSERT_MORE_THAN_ZERO(nDecodedSamples);

    AudioChunkPtr audioChunk;

    if (mResampleContext == 0)
    {
        // Use the plain decoded data without resampling.
        targetData = audioDecodeBuffer;
        audioChunk = boost::make_shared<AudioChunk>(targetData, parameters.getNrChannels(), nDecodedSamples, determinePts(nDecodedSamples));
    }
    else
    {
        // Resample
        typedef boost::rational<int> rational;
        auto convertOutputSampleCountToInputSampleCount = [parameters,codec](int output) -> int
        {
            return
                floor(rational(output) *
                rational(codec->channels) / rational(parameters.getNrChannels()) *
                rational(codec->sample_rate) / rational(parameters.getSampleRate()));
        };
        auto convertInputSampleCountToOutputSampleCount = [parameters,codec](int input) -> int
        {
            return
                floor(rational(input) *
                rational(parameters.getNrChannels()) / rational(codec->channels) *
                rational(parameters.getSampleRate()) / rational(codec->sample_rate));
        };

        int nRemainingInputSamples = nDecodedSamples;
        // The +16 is to compensate for (rounding?) errors I saw. Note that audio_resample sometimes requires multiple passes (which might explain those differences).
        // Choosing this number too low can cause 
        // - ASSERT_MORE_THAN_EQUALS_ZERO(nRemainingOutputSamples) to fail.
        // - Heap corruption errors
        // The number 16 originates from ffmpeg/libavresample/resample.c, where the output length
        // of the resampling is determined by
        //     int lenout= 2 * s->output_channels * nb_samples * s->ratio + 16;
        int nExpectedOutputSamples = convertInputSampleCountToOutputSampleCount(nDecodedSamples) + 16;
        int nRemainingOutputSamples = nExpectedOutputSamples;
        int nResampledSamples = 0;

        // Note: Sometimes audio_resample does not resample the entire buffer in one pass.
        // Here the extra required passes are done, and the data is copied into the pre-allocated audioChunk.
        // The audioChunk is pre-allocated to avoid one extra memcpy (from resampled data into the chunk).

        sample* input = audioDecodeBuffer;

        audioChunk = boost::make_shared<AudioChunk>(static_cast<sample*>(0), parameters.getNrChannels(), nExpectedOutputSamples, determinePts(nExpectedOutputSamples));
        sample* resampled = audioChunk->getBuffer();

        while (nRemainingInputSamples > 0)
        {
            int nInputFrames = nRemainingInputSamples / codec->channels;
            int nOutputFrames = audio_resample(mResampleContext, resampled, input, nInputFrames);
            int nNewOutputSamples = nOutputFrames * parameters.getNrChannels();
            int nUsedInputSamples = convertOutputSampleCountToInputSampleCount(nNewOutputSamples);
            VAR_DEBUG(nInputFrames)(nOutputFrames)(nRemainingOutputSamples)(nNewOutputSamples)(nUsedInputSamples);
            if (nUsedInputSamples == 0)
            {
                break;
            }

            if (nNewOutputSamples > nRemainingOutputSamples)
            {
                // Sometimes more samples than required are generated. Particularly happens when resampling from
                // lowframerate-fewchannels to highframerate-lotsofchannels.
                nNewOutputSamples = nRemainingOutputSamples;
            }
            nRemainingOutputSamples -= nNewOutputSamples;
            ASSERT_MORE_THAN_EQUALS_ZERO(nRemainingOutputSamples);
            nResampledSamples += nNewOutputSamples;
            nRemainingInputSamples -= nUsedInputSamples;
            input += nUsedInputSamples;
            resampled += nNewOutputSamples;
            // NOT: ASSERT_MORE_THAN_EQUALS_ZERO(nRemainingInputSamples)(nUsedInputSamples);
            // WHY: Sometimes audio_resample returns a slightly higher number of output frames then to be expected
            //      by the given amount of input frames (maybe caused by audio_resample mechanism 'caching' some of them in a previous call?).
        }
        // More data was allocated (to compensate for differences between the number of output samples 'calculated' and the number that avcodec actually produced).
        audioChunk->setAdjustedLength(nResampledSamples);

        VAR_DEBUG(nResampledSamples);
    }

    VAR_AUDIO(this)(audioChunk);
    return audioChunk;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void AudioFile::startDecodingAudio(const AudioCompositionParameters& parameters)
{
    if (mDecodingAudio) return;

    // Allocated upon first use. See also the remark in the header file
    // on GCC in combination with make_shared.
    if (!audioDecodeBuffer)
    {
        audioDecodeBuffer   = new sample[sAudioBufferSizeInBytes];
        audioResampleBuffer = new sample[sAudioBufferSizeInBytes];
    }

    startReadingPackets(); // Also causes the file to be opened resulting in initialized avcodec members for File.
    mDecodingAudio = true;

    boost::mutex::scoped_lock lock(Avcodec::sMutex);

    AVCodecContext* codec = getCodec();

    AVCodec* audioCodec = avcodec_find_decoder(codec->codec_id);
    ASSERT_NONZERO(audioCodec);

    int result = avcodec_open2(codec, audioCodec, 0);
    ASSERT_MORE_THAN_EQUALS_ZERO(result);

    int nBytesPerSample = av_get_bytes_per_sample(codec->sample_fmt);
    ASSERT_NONZERO(nBytesPerSample);

    if ((parameters.getNrChannels() != codec->channels) ||
        (parameters.getSampleRate() != codec->sample_rate) ||
        (codec->sample_fmt !=AV_SAMPLE_FMT_S16) ||
        (nBytesPerSample != AudioChunk::sBytesPerSample))
    {
        VAR_INFO(codec->sample_fmt)(parameters.getNrChannels())(codec->channels)(parameters.getSampleRate())(codec->sample_rate);
        static const int taps = 16;
        mResampleContext =
            av_audio_resample_init(
                parameters.getNrChannels(), codec->channels,
                parameters.getSampleRate(), codec->sample_rate,
                AV_SAMPLE_FMT_S16, codec->sample_fmt,
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
        boost::mutex::scoped_lock lock(Avcodec::sMutex);

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