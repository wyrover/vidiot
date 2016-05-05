// Copyright 2013-2016 Eric Raijmakers.
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
#include "FileMetaDataCache.h"
#include "UtilInitAvcodec.h"
#include "UtilPath.h"

namespace model
{
static const int sMicroSecondsPerSeconds = 1000 * 1000;
static const int sMaxBufferSize = 100;

static const int sAudioBufferSizeInBytes = 8192; // Note: always multiple of 32 (align to 32 bytes)

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AudioFile::AudioFile()
    : File()
{
    VAR_DEBUG(*this);
}

AudioFile::AudioFile(const wxFileName& path)
    : File(path, sMaxBufferSize)
{
    VAR_DEBUG(*this);
}

AudioFile::AudioFile(const AudioFile& other)
    : File(other)
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

void AudioFile::moveTo(pts position)
{
    mNewStartPosition.reset(position);

    // position - 1: Ensure that the resulting start position is ALWAYS
    // before the first required sample. Otherwise, seeking sometimes causes
    // a start point beyond the first required sample. That, in turn, causes
    // slight video-audio offset problems and clicks/pops when making cuts
    // within one clip.
    //
    // Typical cases of clicks/pops: make a crossfade/fade in and the samples
    // used within the transition do not align properly with the first samples
    // used AFTER the transition. Same thing can happen when making a cut
    // directly in a clip without any more adjusting.
    //
    // See also AVCodecContext->delay (here, it is assumed that 1 pts value is
    // always greater than the delay value). This has the advantage that the
    // codec need not be opened (getCodec() != 0) for the move operation.
    File::moveTo(std::max<pts>(position - 1,0));
}

void AudioFile::clean()
{
    VAR_DEBUG(this);

    stopDecodingAudio();

    mAudioDecodeBuffer.clear();

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
        model::AudioChunkPtr result{ boost::static_pointer_cast<AudioChunk>(boost::make_shared<EmptyChunk>(parameters.getNrChannels(), parameters.getChunkSize())) };
        result->setError();
        return result;
    }

    PacketPtr audioPacket = getNextPacket();
    samplecount nSkipSamples = 0;


    if (mNewStartPosition)
    {
        pts position = *mNewStartPosition;
        samplecount nextSample = Convert::ptsToSamplesPerChannel(position, getCodec()->sample_rate);

        // Reset the codec. Otherwise, the first audio output after a move is not always the desired sound...
        avcodec_flush_buffers(getCodec());

        auto positionInfoAvailable = [&audioPacket]() -> bool
        {
            return audioPacket && audioPacket->getPacket()->pts != AV_NOPTS_VALUE;
        };

        auto getFirstSampleOfNextPacket = [this](AVPacket* packet) -> samplecount
        {
            if (packet->duration > 0)
            {
                return getFirstSample(packet->pts + packet->duration);
            }
            return getFirstSample(packet->pts + 1);
        };

        while (positionInfoAvailable() &&
            getFirstSampleOfNextPacket(audioPacket->getPacket()) <= nextSample)
        {
            // The next packet starts also 'before' the required sample. Use that packet.
            // Seeking was too far ahread of the required point.
            audioPacket = getNextPacket();
        }

        if (positionInfoAvailable())
        {
            samplecount firstSample = getFirstSample(audioPacket->getPacket()->pts);
            if (firstSample > nextSample)
            {
                // For some file types the 'sample seeking algorithm' does not work.
                // For instance, when moving to the beginning of a file, the first packet
                // returned does not have pts value '0'.
                LOG_WARNING << "Audio reposition failure [" << getDescription() << "][" << getCodec() << "][" << audioPacket->getPacket() << ']';
            }
            else
            {
                nSkipSamples = nextSample - firstSample;
            }
        }

        mNewStartPosition.reset();
    }

    ASSERT_MORE_THAN_EQUALS_ZERO(nSkipSamples);
    nSkipSamples *= parameters.getNrChannels();

    //////////////////////////////////////////////////////////////////////////
    // DECODING

    AVCodecContext* codec = getCodec();

    auto allocateFrame = []() -> boost::shared_ptr<AVFrame>
    {
        // Ensure memory cleaned up in all possible control flows:

        boost::shared_ptr<AVFrame> frame(av_frame_alloc(), [](AVFrame* frame) { av_frame_free(&frame); });
        ASSERT_NONZERO(frame);
        ASSERT_NONZERO(frame.get());
        return frame;
    };

    auto copyFrame = [this, codec](boost::shared_ptr<AVFrame> frame, int offset) -> int
    {
        int decodedLineSize(0); // Will contain the number of bytes per plane
        // return value of av_samples_get_buffer_size (unused) contains the entire required buffer size:
        // - Contains ALL channel data
        // - Contains 32 bit alignment for all used data fields (one for packet, multiple for planar)
        av_samples_get_buffer_size(&decodedLineSize, codec->channels, frame->nb_samples, codec->sample_fmt, 1);

        size_t requiredBufferSize{ static_cast<size_t>(offset + decodedLineSize) };
        if (requiredBufferSize > mAudioDecodeBuffer[0].size())
        {
            // Some file formats packets (.ape) contain multiple sizes.
            // First seen packet may not be the biggest...
            int size{ offset + decodedLineSize };
            int newAudioBufferSizeInBytes{ (size + 32 - 1) - (size + 32 - 1) % 32 }; // Align to 32 bytes
            for (int i = 0; i < mNrPlanes; ++i)
            {
                mAudioDecodeBuffer[i].resize(newAudioBufferSizeInBytes);
            }
        }
        ASSERT_LESS_THAN_EQUALS(requiredBufferSize, mAudioDecodeBuffer[0].size())(offset)(decodedLineSize)(*this);
        for (int i = 0; i < mNrPlanes; ++i)
        {
            if (frame->extended_data[i] != 0)
            {
                // Only take planes with actual data into account.
                // Example: resolutionchange_contains_6_audio_planes_but_not_for_all_packets.mpg
                //          This file contains (according to the meta data from the AVCodecContext)
                //          6 audio planes. However, for some packets in the audio stream there are
                //          only two contained planes.
                memcpy(&(mAudioDecodeBuffer[i][offset]), frame->extended_data[i], decodedLineSize);
            }
            else
            {
                memset(&(mAudioDecodeBuffer[i][offset]), 0, decodedLineSize);
            }
        }
        return decodedLineSize;
    };


    // All sizes are in bytes below
    int targetSizeInBytes = 0; // For planar data, size of each plane. For packet data, size of first plane, which contains all channels.
    int nDecodedSamplesPerChannel = 0;
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

            boost::shared_ptr<AVFrame> frame{ allocateFrame() };

            int got_frame = 0;
            int usedSourceBytes = avcodec_decode_audio4(codec, frame.get(), &got_frame, &packet);

            if (usedSourceBytes < 0 || !got_frame)
            {
                audioPacket = getNextPacket();
                break; // This frame failed. Can happen after moveTo(); due to seeking the first packets do not contain (enough) header information.
            }
            else
            {
                // Some data was decoded.
                done = true;
            }

            int decodedLineSize{ copyFrame(frame, targetSizeInBytes) };  // decodedLineSize will contain the number of bytes per plane

            sourceData += usedSourceBytes;
            sourceSize -= usedSourceBytes;

            targetSizeInBytes += decodedLineSize;
            nDecodedSamplesPerChannel += frame->nb_samples;

            // Only after the first packet has been decoded, all of the information
            // required for initializing resampling is available.
            if (mNeedsResampling && mSoftwareResampleContext == 0)
            {
                // Code taken from ffplay.c
                int64_t dec_channel_layout =
                    (frame->channel_layout && av_frame_get_channels(frame.get()) == av_get_channel_layout_nb_channels(frame->channel_layout)) ?
                    frame->channel_layout : av_get_default_channel_layout(av_frame_get_channels(frame.get()));

                // Only take planes with actual data into account.
                // Some files have packets with fewer frames than described in the codec.
                int nPlanesInFrame{ 0 };
                for (int i = 0; i < mNrPlanes; ++i)
                {
                    if (frame->extended_data[i] != 0)
                    {
                        nPlanesInFrame++;
                    }
                }
                if (nPlanesInFrame != mNrPlanes)
                {
                    dec_channel_layout = av_get_default_channel_layout(nPlanesInFrame);
                }

                mSoftwareResampleContext = swr_alloc_set_opts(0,
                    av_get_default_channel_layout(parameters.getNrChannels()),
                    AV_SAMPLE_FMT_S16,
                    Convert::samplerateToNewSpeed(parameters.getSampleRate(), parameters.getSpeed(), 1),
                    dec_channel_layout, codec->sample_fmt, frame->sample_rate, 0, 0);
                ASSERT_NONZERO(mSoftwareResampleContext);

                int result { swr_init(mSoftwareResampleContext) };
                ASSERT_ZERO(result)(avcodecErrorString(result));
            }
        }
    }

    if (!done) // audioPacket == nullptr: flush with 0 packets until no more data returned
    {
        AVPacket packet;
        memset(&packet, 0, sizeof(packet));

        boost::shared_ptr<AVFrame> frame{ allocateFrame() };

        int got_frame = 0;
        int usedSourceBytes = avcodec_decode_audio4(codec, frame.get(), &got_frame, &packet);

        if (!got_frame)
        {
            // No samples, end of data
            static const std::string status("End of file");
            VAR_DEBUG(status);
            return AudioChunkPtr();
        }

#pragma GCC diagnostic ignored "-Wunused-variable"
        int decodedLineSize{ copyFrame(frame, targetSizeInBytes) };  // decodedLineSize will contain the number of bytes per plane

        nDecodedSamplesPerChannel += frame->nb_samples;

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
        audioChunk = boost::make_shared<AudioChunk>(parameters.getNrChannels(), nDecodedSamplesPerChannel * parameters.getNrChannels(), true, false, reinterpret_cast<sample*>(&(mAudioDecodeBuffer[0][0])));
    }
    else
    {
        // Resample
        auto convertInputSampleCountToOutputSampleCount = [parameters,codec](samplecount input) -> samplecount
        {
            return floor(rational64(input) * rational64(Convert::samplerateToNewSpeed(parameters.getSampleRate(), parameters.getSpeed(), 1)) / rational64(codec->sample_rate));
        };

        int nExpectedOutputSamplesPerChannel = convertInputSampleCountToOutputSampleCount(nDecodedSamplesPerChannel);
        nExpectedOutputSamplesPerChannel = std::max(nExpectedOutputSamplesPerChannel, swr_get_out_samples(mSoftwareResampleContext, nDecodedSamplesPerChannel));
        int bufferSize = av_samples_get_buffer_size(0, parameters.getNrChannels(), nExpectedOutputSamplesPerChannel, AV_SAMPLE_FMT_S16, 1);
        int nExpectedOutputFrames = swr_get_out_samples(mSoftwareResampleContext, nDecodedSamplesPerChannel);
        // The audioChunk is pre-allocated to avoid one extra memcpy (from resampled data into the chunk).
        audioChunk = boost::make_shared<AudioChunk>(parameters.getNrChannels(), bufferSize, true, false);

        uint8_t *out[1]; // Output data always packed into one plane
        out[0] = reinterpret_cast<uint8_t*>(audioChunk->getBuffer());
        uint8_t **in{ new uint8_t*[mNrPlanes] };
        for (int i{ 0 }; i < mNrPlanes; ++i)
        {
            in[i] = reinterpret_cast<uint8_t*>(&(mAudioDecodeBuffer[i][0]));
        }
        int nOutputFrames = swr_convert(mSoftwareResampleContext, &out[0], bufferSize, const_cast<const uint8_t**>(&in[0]), nDecodedSamplesPerChannel);
        ASSERT_MORE_THAN_EQUALS_ZERO(nOutputFrames);
        delete[] in;
        int nOutputSamples = nOutputFrames * parameters.getNrChannels();

        // To check that the buffer was large enough to hold everything produced by swr_convert in one pass.
        ASSERT_LESS_THAN_EQUALS(nOutputSamples, bufferSize);

        // More data was allocated (to compensate for differences between the number
        // of output samples 'calculated' and the number that avcodec actually produced).
        audioChunk->setAdjustedLength(nOutputSamples);

        // NOTE: When splitting a clip into several smaller parts, the splitting may cause the total sum of
        //       returned samples by swr_convert to differ slightly (order of 1 or 2 samples difference per
        //       decoded packet. So, computing the sum of all returned output samples of a clip split into
        //       parts is not exactly the same as the returned output samples of the large still joined clip.
        //
        // NOTE: The std min is required because sometimes nOutputSamples is slightly smaller than the skipped
        //       samples.
        nSkipSamples = std::min<samplecount>(convertInputSampleCountToOutputSampleCount(nSkipSamples), nOutputSamples);
    }

    ASSERT_MORE_THAN_EQUALS_ZERO(nSkipSamples);
    audioChunk->read(nSkipSamples);

    return audioChunk;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

int AudioFile::getSampleRate()
{
    return getCodec()->sample_rate;
}

int AudioFile::getChannels()
{
    return getCodec()->channels;
}

boost::optional<pts> AudioFile::getNewStartPosition() const
{
    return mNewStartPosition;
}

AudioPeaks AudioFile::getPeaks(const AudioCompositionParameters& parameters, pts offset, pts length) // todo add speed here!
{
    if (!canBeOpened())
    {
        return AudioPeaks();
    }
    ASSERT_MORE_THAN_EQUALS_ZERO(offset);
    ASSERT_MORE_THAN_EQUALS_ZERO(length);

    boost::optional<AudioPeaks> peaks{ FileMetaDataCache::get().getPeaks(getPath()) };

    if (!peaks)
    {
        // The setPts() & determineChunkSize() below is required for the case where the file has been removed from disk,
        // and the chunk size is used to initialize a chunk of silence.
        moveTo(0);

        AudioPeaks allPeaks;
        AudioPeak current{ 0, 0 };
        samplecount samplePosition{ 0 };
        samplecount nextRequiredSample{ 0 };
        AudioChunkPtr chunk{ getNextAudio(parameters) };

        size_t data_length{ narrow_cast<size_t>(getLength()) };

        while (chunk && allPeaks.size() < data_length)
        {
            samplecount chunksize = chunk->getUnreadSampleCount();
            sample* buffer = chunk->getBuffer();

            for (int i = 0; (i < chunksize) && (allPeaks.size() < data_length); ++i)
            {
                current.first = std::min(current.first, *buffer);
                current.second = std::max(current.second, *buffer);
                if (samplePosition == nextRequiredSample)
                {
                    ASSERT_LESS_THAN_EQUALS_ZERO(current.first);
                    ASSERT_MORE_THAN_EQUALS_ZERO(current.second);
                    allPeaks.emplace_back(current);
                    current = AudioPeak(0, 0);
                    nextRequiredSample = Convert::ptsToSamplesPerChannel(Convert::samplerateToNewSpeed(parameters.getSampleRate(), parameters.getSpeed(), 1), allPeaks.size());
                }
                ++samplePosition;
                ++buffer;
            }
            chunk = getNextAudio(parameters);
        }
        FileMetaDataCache::get().setPeaks(getPath(), allPeaks);
        peaks.reset(allPeaks);
    }

    const AudioPeaks& allPeaks{ *peaks };
    pts total{ allPeaks.size() };

    AudioPeaks result;

    // NOT: ASSERT_LESS_THAN_EQUALS(narrow_cast<size_t>(offset), allPeaks.size())(*this);
    // ==> Sometimes the number of peaks equals '0' at this point (decoding error?). Do not crash but use 'empty' peaks.
    //
    // NOT: ASSERT_LESS_THAN_EQUALS(offset + length, allPeaks.size())(*this);
    // ==> See also  AudioClip::getNextAudio where sometimes extra audio is added, if the audio data length in a file is smaller than the audio length.
    //
    // The audio clip may be slightly larger than the audio file data. This can be caused by the clip having (typically) the same length as a linked video clip.
    // The video data in a file may be slightly longer than the audio data, resulting in such a difference. Instead of truncating the video, the audio is extended
    // with silence, leaving the truncating (the choice) to the user.

    if (offset < total)
    {
        auto itBegin = allPeaks.begin() + offset;
        auto itEnd = allPeaks.begin() + std::min(total, offset + length);
        if ((std::distance(allPeaks.begin(), itBegin) < total) &&
            (std::distance(allPeaks.begin(), itEnd) <= total))
        {
            result = std::move(AudioPeaks(itBegin, itEnd));
        }
    }
    
    if (result.size() != narrow_cast<size_t>(length))
    {
        // Ensure resulting peaks length equals length of clip. Add 'silence' if required.
        result.resize(length,std::make_pair(0,0));
    }
    return result;
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

    mAudioDecodeBuffer = std::vector< std::vector<uint8_t> >(mNrPlanes, std::vector<uint8_t>(sAudioBufferSizeInBytes));

    AVCodec* audioCodec = avcodec_find_decoder(codec->codec_id);
    ASSERT_NONZERO(audioCodec);

    boost::mutex::scoped_lock lock(Avcodec::sMutex);

    int result = avcodec_open2(codec, audioCodec, 0);
    ASSERT_MORE_THAN_EQUALS_ZERO(result)(avcodecErrorString(result));

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

        mAudioDecodeBuffer.clear();
    }
    mDecodingAudio = false;
}

samplecount AudioFile::getFirstSample(int64_t pts)
{
    AVStream* stream{ getStream() };
    ASSERT_NONZERO(stream);
    // Use the real start position of the used audio data. This ensures audio-video sync
    // in case the audio and video have different start positions.
    pts -= getStreamStartPosition();
    samplecount result =
        floor(
        rational64(pts) *
        rational64(stream->time_base.num, stream->time_base.den) *
        rational64(getCodec()->sample_rate));
    return result;
}

//////////////////////////////////////////////////////////////////////////
// FROM FILE
//////////////////////////////////////////////////////////////////////////

bool AudioFile::useStream(const AVMediaType& type) const
{
    return (type == AVMEDIA_TYPE_AUDIO);
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const AudioFile& obj)
{
    os << static_cast<const File&>(obj) << '|'
        << obj.mDecodingAudio << '|'
        << obj.mNeedsResampling << '|'
        << obj.mSoftwareResampleContext << '|'
        << obj.mNrPlanes;
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
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void AudioFile::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void AudioFile::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::AudioFile)
