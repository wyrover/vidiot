// Copyright 2013,2014 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#include "AudioFile.h"

#include "AudioChunk.h"
#include "AudioCompositionParameters.h"
#include "Convert.h"
#include "EmptyChunk.h"
#include "UtilInitAvcodec.h"
#include "UtilLog.h"
#include "UtilLogAvcodec.h"

namespace model
{
static const int sMicroSecondsPerSeconds = 1000 * 1000;
static const int sMaxBufferSize = 100;

static const int sAudioBufferSizeInBytes = 192000;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AudioFile::AudioFile()
    :	File()
    ,   mSoftwareResampleContext (0)
    ,   mDecodingAudio(false)
    ,   mNeedsResampling(false)
    ,   audioDecodeBuffer(0)
    ,   mNrPlanes(0)
{
    VAR_DEBUG(*this);
}

AudioFile::AudioFile(const wxFileName& path)
    :	File(path,sMaxBufferSize)
    ,   mSoftwareResampleContext (0)
    ,   mDecodingAudio(false)
    ,   mNeedsResampling(false)
    ,   audioDecodeBuffer(0)
    ,   mNrPlanes(0)
{
    VAR_DEBUG(*this);
}

AudioFile::AudioFile(const AudioFile& other)
    :   File(other)
    ,   mSoftwareResampleContext (0)
    ,   mDecodingAudio(false)
    ,   mNeedsResampling(false)
    ,   audioDecodeBuffer(0)
    ,   mNrPlanes(0)
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
    clean();
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

void AudioFile::clean()
{
    VAR_DEBUG(this);

    stopDecodingAudio();

    if (audioDecodeBuffer)
    {
        for (int i = 0; i < mNrPlanes; ++i)
        {
            delete[] audioDecodeBuffer[i];
        }
        delete[] audioDecodeBuffer;
    }
    audioDecodeBuffer = 0;

    File::clean();
}

//////////////////////////////////////////////////////////////////////////
// IAUDIO
//////////////////////////////////////////////////////////////////////////

AudioChunkPtr AudioFile::getNextAudio(const AudioCompositionParameters& parameters)
{
    startDecodingAudio(parameters);

    if (!canBeOpened())
    {
        // If file could not be read (for whatever reason) return empty audio.
        return boost::static_pointer_cast<AudioChunk>(boost::make_shared<EmptyChunk>(parameters.getNrChannels(), parameters.ptsToSamples(1)));
    }

    PacketPtr audioPacket = getNextPacket();

    //////////////////////////////////////////////////////////////////////////
    // DECODING

    AVCodecContext* codec = getCodec();

    // All sizes are in bytes below
    int targetSizeInBytes = 0; // For planar data, size of each plane. For packet data, size of first plane, which contains all channels.
    int nDecodedSamplesPerChannel = 0;

    if (audioPacket)
    {
        bool done = false;

        while (!done && audioPacket)
        {
            uint8_t* sourceData = audioPacket->getPacket()->data;
            int sourceSize = audioPacket->getPacket()->size;
            ASSERT_MORE_THAN_ZERO(sourceSize);

            while (sourceSize > 0)
            {
                AVPacket packet;
                memset(&packet, 0, sizeof(packet));
                packet.data = sourceData;
                packet.size = sourceSize;

                AVFrame* pFrame = av_frame_alloc();
                ASSERT_NONZERO(pFrame);

                int got_frame = 0;
                int usedSourceBytes = avcodec_decode_audio4(codec, pFrame, &got_frame, &packet);
                
                if (usedSourceBytes < 0)
                {
                    audioPacket = getNextPacket();
                    break; // This frame failed. Can happen after moveTo(); due to seeking the first packets do not contain (enough) header information.
                }
                else
                {
                    // Some data was decoded.
                    done = true;
                }

                if (!got_frame)
                {
                    sourceSize = 0;
                    break;
                }

                int decodedLineSize(0); // Will contain the number of bytes per plane
                // decodeBuferInBytes contains the entire required buffer size:
                // - Contains ALL channel data
                // - Contains 32 bit alignment for all used data fields (one for packet, multiple for planar)
                int decodeBufferInBytes = av_samples_get_buffer_size(&decodedLineSize, codec->channels, pFrame->nb_samples, codec->sample_fmt, 1);
                for (int i = 0; i < mNrPlanes; ++i)
                {
                    memcpy(audioDecodeBuffer[i] + targetSizeInBytes, pFrame->extended_data[i], decodedLineSize);
                }

                sourceData += usedSourceBytes;
                sourceSize -= usedSourceBytes;

                targetSizeInBytes += decodedLineSize;
                nDecodedSamplesPerChannel += pFrame->nb_samples;

                // Only after the first packet has been decoded, all of the information
                // required for initializing resampling is available.
                if (mNeedsResampling && mSoftwareResampleContext == 0)
                {
                    // Code taken from ffplay.c
                    int64_t dec_channel_layout =
                        (pFrame->channel_layout && av_frame_get_channels(pFrame) == av_get_channel_layout_nb_channels(pFrame->channel_layout)) ?
                        pFrame->channel_layout : av_get_default_channel_layout(av_frame_get_channels(pFrame));

                    mSoftwareResampleContext = swr_alloc_set_opts(0,
                        av_get_default_channel_layout(parameters.getNrChannels()), AV_SAMPLE_FMT_S16, parameters.getSampleRate(),
                        dec_channel_layout, codec->sample_fmt, pFrame->sample_rate, 0, 0);
                    ASSERT_NONZERO(mSoftwareResampleContext);

                    int result = swr_init(mSoftwareResampleContext);
                    ASSERT_ZERO(result)(avcodecErrorString(result));
                }

                av_frame_free(&pFrame);
            }
        }
    }
    else // !audioPacket: flush with 0 packets until no more data returned
    {
        AVPacket packet;
        memset(&packet, 0, sizeof(packet));

        AVFrame frame = { { 0 } };
        int got_frame = 0;
        int usedSourceBytes = avcodec_decode_audio4(codec, &frame, &got_frame, &packet);

        if (!got_frame)
        {
            // No samples, end of data
            static const std::string status("End of file");
            VAR_DEBUG(status);
            return AudioChunkPtr();
        }

        int decodedLineSize(0); // Will contain the number of bytes per plane
        // decodeBuferInBytes contains the entire required buffer size:
        // - Contains ALL channel data
        // - Contains 32 bit alignment for all used data fields (one for packet, multiple for planar)
        int decodeBufferInBytes = av_samples_get_buffer_size(&decodedLineSize, codec->channels, frame.nb_samples, codec->sample_fmt, 1);
        for (int i = 0; i < mNrPlanes; ++i)
        {
            memcpy(audioDecodeBuffer[i], frame.extended_data[i], decodedLineSize);
        }

        targetSizeInBytes += decodedLineSize;
        nDecodedSamplesPerChannel += frame.nb_samples;

        ASSERT_IMPLIES(mNeedsResampling, mSoftwareResampleContext != 0); // Must have been initialized already
    }

    ASSERT_MORE_THAN_ZERO(nDecodedSamplesPerChannel);

    //////////////////////////////////////////////////////////////////////////
    // RESAMPLING

    AudioChunkPtr audioChunk;

    if (mSoftwareResampleContext == 0)
    {
        // Use the plain decoded data without resampling.
        ASSERT_EQUALS(mNrPlanes,1); // Resulting data is never planar, in case of planar data resampling should be done
        audioChunk = boost::make_shared<AudioChunk>(parameters.getNrChannels(), nDecodedSamplesPerChannel * parameters.getNrChannels(), true, false, (sample*)audioDecodeBuffer[0]);
    }
    else
    {
        // Resample
        typedef boost::rational<int> rational;
        auto convertInputSampleCountToOutputSampleCount = [parameters,codec](int input) -> int
        {
            return floor(rational(input) * rational(parameters.getSampleRate()) / rational(codec->sample_rate));
        };

        int nExpectedOutputSamplesPerChannel = convertInputSampleCountToOutputSampleCount(nDecodedSamplesPerChannel);

        int bufferSize = av_samples_get_buffer_size(0, parameters.getNrChannels(), nExpectedOutputSamplesPerChannel, AV_SAMPLE_FMT_S16, 1);

        // The audioChunk is pre-allocated to avoid one extra memcpy (from resampled data into the chunk).
        audioChunk = boost::make_shared<AudioChunk>(parameters.getNrChannels(), bufferSize, true, false);

        uint8_t *out[1]; // Output data always packet into one plane
        out[0] = reinterpret_cast<uint8_t*>(audioChunk->getBuffer());
        int nOutputFrames = swr_convert(mSoftwareResampleContext, &out[0], bufferSize, const_cast<const uint8_t**>(audioDecodeBuffer), nDecodedSamplesPerChannel);
        ASSERT_MORE_THAN_EQUALS_ZERO(nOutputFrames);
        int nOutputSamples = nOutputFrames * parameters.getNrChannels();
        // To check that the buffer was large enough to hold everything produced by swr_convert in one pass.
        ASSERT_LESS_THAN(nOutputSamples, bufferSize);

        // More data was allocated (to compensate for differences between the number
        // of output samples 'calculated' and the number that avcodec actually produced).
        audioChunk->setAdjustedLength(nOutputSamples);
    }

    return audioChunk;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

int AudioFile::getSampleRate()
{
    return getCodec()->sample_rate;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void AudioFile::startDecodingAudio(const AudioCompositionParameters& parameters)
{
    if (mDecodingAudio) return;

    startReadingPackets(); // Also causes the file to be opened resulting in initialized avcodec members for File.

    if (!canBeOpened()) { return; } // File could not be opened (deleted?)

    mDecodingAudio = true;

    AVCodecContext* codec = getCodec();

    mNrPlanes = av_sample_fmt_is_planar(codec->sample_fmt) ? codec->channels : 1;

    // Allocated upon first use. See also the remark in the header file
    // on GCC in combination with make_shared.
    if (!audioDecodeBuffer)
    {
        audioDecodeBuffer = new uint8_t*[mNrPlanes];
        for (int i = 0; i < mNrPlanes; ++i)
        {
            audioDecodeBuffer[i] = new uint8_t[sAudioBufferSizeInBytes];
        }
    }

    AVCodec* audioCodec = avcodec_find_decoder(codec->codec_id);
    ASSERT_NONZERO(audioCodec);

    boost::mutex::scoped_lock lock(Avcodec::sMutex);

    int result = avcodec_open2(codec, audioCodec, 0);
    ASSERT_MORE_THAN_EQUALS_ZERO(result);

    int nBytesPerSample = av_get_bytes_per_sample(codec->sample_fmt);
    if ((parameters.getNrChannels() != codec->channels) ||
        (parameters.getSampleRate() != codec->sample_rate) ||
        (codec->sample_fmt != AV_SAMPLE_FMT_S16) ||
        (nBytesPerSample != AudioChunk::sBytesPerSample))
    {
        mNeedsResampling = true;
    }

    ASSERT_ZERO(mSoftwareResampleContext);

    VAR_DEBUG(this)(getCodec());
}

void AudioFile::stopDecodingAudio()
{
    VAR_DEBUG(this);
    if (mDecodingAudio)
    {
        boost::mutex::scoped_lock lock(Avcodec::sMutex);

        if (mSoftwareResampleContext != 0)
        {
            LOG_INFO << "Resampling ended";
            swr_free(&mSoftwareResampleContext);
        }

        avcodec_close(getCodec());
    }
    mDecodingAudio = false;
}

//////////////////////////////////////////////////////////////////////////
// FROM FILE
//////////////////////////////////////////////////////////////////////////

bool AudioFile::useStream(const AVMediaType& type) const
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

std::ostream& operator<<(std::ostream& os, const AudioFile& obj)
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
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(File);
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(IAudio);
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void AudioFile::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void AudioFile::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::AudioFile)