// Copyright 2013-2015 Eric Raijmakers.
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

#include "Render.h"

#include "AudioChunk.h"
#include "AudioCodec.h"
#include "AudioCodecs.h"
#include "AudioCompositionParameters.h"
#include "Config.h"
#include "Constants.h"
#include "Convert.h"
#include "Dialog.h"
#include "EmptyFrame.h"
#include "Folder.h"
#include "OutputFormat.h"
#include "OutputFormats.h"
#include "Project.h"
#include "Properties.h"
#include "Sequence.h"
#include "StatusBar.h"
#include "UtilClone.h"
#include "UtilLog.h"
#include "UtilLogAvcodec.h"
#include "UtilLogWxwidgets.h"
#include "UtilPath.h"
#include "UtilSerializeBoost.h"
#include "UtilSerializeWxwidgets.h"
#include "VideoCodec.h"
#include "VideoCodecs.h"
#include "VideoCompositionParameters.h"
#include "VideoFrame.h"
#include "Work.h"
#include "Worker.h"

namespace model { namespace render {

static AVFrame *alloc_picture(const enum PixelFormat& pix_fmt, int width, int height);

//////////////////////////////////////////////////////////////////////////
// RENDERING
//////////////////////////////////////////////////////////////////////////

class RenderWork : public worker::Work
{
public:
    RenderWork(const SequencePtr& sequence, pts from, pts to)
    :   worker::Work(std::bind(&RenderWork::generate,this))
    ,   mSequence(sequence)
    ,   mFrom(from)
    {
        pts maxLength = Convert::timeToPts(Config::ReadLong(Config::sPathDebugMaxRenderLength) *  Constants::sSecond);
        mLength = to - from;
        if (maxLength > 0 && maxLength < mLength)
        {
            mLength = maxLength;
        }
        VAR_INFO(sequence)(from)(to)(mFrom)(mLength);
        ASSERT_MORE_THAN_EQUALS_ZERO(mFrom);
        ASSERT_MORE_THAN_ZERO(mLength);
        ASSERT_LESS_THAN_EQUALS(mFrom,sequence->getLength());
        ASSERT_LESS_THAN_EQUALS(mFrom + mLength,sequence->getLength());
    }

    void generate();

private:

    model::SequencePtr mSequence;
    pts mFrom;
    pts mLength;
};

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

// static
void Render::initialize()
{
    VideoCodecs::initialize();
    AudioCodecs::initialize();
    OutputFormats::initialize();
}

Render::Render()
    :   wxEvtHandler()
    ,   mFileName("/")
    ,   mOutputFormat(OutputFormats::getDefault())
    ,   mSeparateAtCuts(false)
    ,   mStart(0)
    ,   mEnd(0)
{
    VAR_DEBUG(this);

}

Render::Render(const Render& other)
    :   wxEvtHandler()
    ,   mFileName(other.mFileName)
    ,   mOutputFormat(make_cloned<OutputFormat>(other.mOutputFormat))
    ,   mSeparateAtCuts(other.mSeparateAtCuts)
    ,   mStart(other.mStart)
    ,   mEnd(other.mEnd)
{
    VAR_DEBUG(this);
}

Render* Render::clone() const
{
    return new Render(static_cast<const Render&>(*this));
}

void Render::onCloned()
{
}

Render::~Render()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// OPERATORS
//////////////////////////////////////////////////////////////////////////

bool Render::operator== (const Render& other) const
{
    return
        (mFileName == other.mFileName) &&
        (*mOutputFormat == *other.mOutputFormat);
}

bool Render::operator!= (const Render& other) const
{
    return (!operator==(other));
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

OutputFormatPtr Render::getOutputFormat() const
{
    return mOutputFormat;
}

void Render::setOutputFormat(const OutputFormatPtr& format)
{
    mOutputFormat = format;
}

wxFileName Render::getFileName() const
{
    return mFileName;
}

void Render::setFileName(const wxFileName& filename)
{
    mFileName = filename;
}

bool Render::getSeparateAtCuts() const
{
    return mSeparateAtCuts;
}

void Render::setSeparateAtCuts(bool separate)
{
    mSeparateAtCuts = separate;
}

bool Render::checkFileName() const
{
    return Render::checkFileName(mFileName);
}

// static
bool Render::checkFileName(const wxFileName& filename)
{
    if (!filename.IsOk())
    {
        VAR_ERROR(filename);
        return false;
    }
    if (filename.IsDir())
    {
        VAR_ERROR(filename);
        return false;
    }
    if (!filename.HasExt())
    {
        VAR_ERROR(filename);
        return false;
    }
    if (!filename.HasName())
    {
        VAR_ERROR(filename);
        return false;
    }
    if (filename.FileExists() && !filename.IsFileWritable())
    {
        VAR_ERROR(filename);
        return false;
    }
    return true;
}

RenderPtr Render::withFileNameRemoved() const
{
    RenderPtr cloned = RenderPtr(clone());
    wxFileName filename = cloned->getFileName();
    // Extension is not cleared (the default extension is stored this way)
    filename.SetName("");
    cloned->setFileName(filename);
    return cloned;
}

//////////////////////////////////////////////////////////////////////////
// SCHEDULING
//////////////////////////////////////////////////////////////////////////

// static
void Render::schedule(const SequencePtr& sequence)
{
    if (!sequence->getRender()->getSeparateAtCuts())
    {
        model::SequencePtr clone = make_cloned<model::Sequence>(sequence);
        worker::VisibleWorker::get().schedule(boost::make_shared<RenderWork>(clone,0,clone->getLength()));
    }
    else
    {
        std::set<pts> cuts = sequence->getCuts();
        if (cuts.size() < 2) { return; }
        std::set<pts>::iterator it = cuts.begin();
        pts start = *it;
        ++it;
        int c = 1;
        while (it != cuts.end() )
        {
            pts end = *it;

            if (!sequence->isEmptyAt(start))
            {
                model::SequencePtr clone = make_cloned<model::Sequence>(sequence);
                wxFileName fn = clone->getRender()->getFileName();
                fn.SetName(fn.GetName() + wxString::Format("_%d",c));
                clone->getRender()->setFileName(fn);
                worker::VisibleWorker::get().schedule(boost::make_shared<RenderWork>(clone,start,end));
                ++c;
            }
            // else: If there's only empty space at this interval's begin point: do not render the empty space and skip to the next interval.
            ++it;
            start = end;
        }
    }
}

typedef std::vector<SequencePtr> Sequences;
void findSequences(const FolderPtr& node, Sequences& result)
{
    for ( NodePtr child : node->getChildren() )
    {
        if (child->isA<Sequence>())
        {
            result.push_back(boost::dynamic_pointer_cast<Sequence>(child));
        }
        else if (child->isA<Folder>())
        {
            findSequences(boost::dynamic_pointer_cast<Folder>(child),result);
        }
    }
}

// static
void Render::scheduleAll()
{
    Sequences seqs;
    findSequences(Project::get().getRoot(),seqs);

    bool anError = false;
    wxString error;
    error << _("The following sequence(s) have not been scheduled:\n");
    wxStrings errors;

    wxFileNames allFilenames;
    for ( SequencePtr sequence : seqs )
    {
        allFilenames.push_back(sequence->getRender()->getFileName());
    }

    for ( SequencePtr sequence : seqs )
    {
        if (!sequence->getRender()->checkFileName())
        {
            error << "- " << sequence->getName() << ": Filename is not specified.\n";
            anError = true;
            continue;
        }
        if (std::count(allFilenames.begin(),allFilenames.end(),sequence->getRender()->getFileName()) > 1)
        {
            error << "- " << sequence->getName() << ": Specified filename also used for another sequence.\n";
            anError = true;
            continue;
        }
        schedule(sequence);
    }
    if (anError)
    {
        gui::Dialog::get().getConfirmation(_("Not all sequences have been scheduled:"), error);
    }
}

//////////////////////////////////////////////////////////////////////////
// RENDERING
//////////////////////////////////////////////////////////////////////////

struct EncodingError : std::exception
{
    explicit EncodingError(wxString _message)
        : std::exception()
        , message(_message)
    {
    }

    char const* what() const throw() { return message; }

    friend
    std::ostream& operator<<(std::ostream& os, const EncodingError& obj)
    {
        os << obj.message;
        return os;
    }

    wxString message;
};

void RenderWork::generate()
{
    VAR_INFO(this);
    setThreadName("RenderWork::generate");
    model::SequencePtr sequence = mSequence;
    pts position = mFrom;
    RenderPtr mRender = sequence->getRender();

    sequence->moveTo(mFrom);
    showProgressBar(mLength, true);
    wxString ps; ps << _("Rendering sequence '") << sequence->getName() << "'";
    showProgressText(ps);

    OutputFormatPtr outputformat = mRender->getOutputFormat();

    AVFormatContext* context = outputformat->getContext();
    ASSERT(mRender->getFileName().IsOk());
    ASSERT_NONZERO(context);
    wxString filename = mRender->getFileName().GetFullPath();
    ASSERT_LESS_THAN_EQUALS(sizeof(filename.c_str()),sizeof(context->filename));
    #ifdef _MSC_VER
    _snprintf(context->filename, sizeof(context->filename), "%s", filename.c_str());
    #else
    strncpy(context->filename, filename.c_str().AsChar(), sizeof(context->filename));
    #endif

    bool storeAudio = outputformat->storeAudio();
    bool storeVideo = outputformat->storeVideo();
    ASSERT(storeAudio || storeVideo)(storeAudio)(storeVideo);

    double lengthInSeconds = Convert::ptsToSeconds(mLength);

    bool fileOpened = false;

    bool videoOpened = false;
    bool audioOpened = false;

    bool videoEnd = false; // Video end seen
    bool audioEnd = false; // Audio end seen

    AVStream* videoStream = 0;
    AVStream* audioStream = 0;

    AVCodecContext* videoCodec = 0;
    AVCodecContext* audioCodec = 0;

    AVFrame* outputPicture = 0;
    AVFrame* colorSpaceConversionPicture = 0;
    struct SwsContext *colorSpaceConversionContext = 0;

    sample* samples = 0;
    samplecount nRequiredInputSamplesPerChannel = 0;
    samplecount nRequiredInputSamplesForAllChannels = 0;
    int bytesPerSampleForEncoder = 0;
    SwrContext* audioSampleFormatResampleContext = 0;
    int nAudioPlanes = 0;
    uint8_t** resampledAudioData = 0;
    samplecount fedSamples = 0;

    AVRational sampleTimeBase; // sampleTimeBase == 1 / audio sample rate
    sampleTimeBase.num = 1;
    sampleTimeBase.den = model::Properties::get().getAudioSampleRate();

    boost::shared_ptr<VideoCompositionParameters> videoParameters = boost::make_shared<VideoCompositionParameters>();
    boost::shared_ptr<AudioCompositionParameters> audioParameters = boost::make_shared<AudioCompositionParameters>();

    try
    {

        //////////////////////////////////////////////////////////////////////////
        // OPEN STREAMS
        //////////////////////////////////////////////////////////////////////////

        if (storeVideo)
        {
            videoStream = outputformat->getVideoCodec()->addStream(context);
            videoCodec = videoStream->codec;
            if (videoCodec->ticks_per_frame != 1)
            {
                VAR_ERROR(videoCodec->ticks_per_frame)(videoCodec);
                throw EncodingError("Unsupported number of ticks per frame in target codec.");
            }

            if (Config::Exists(Config::sPathVideoOverruleFourCC))
            {
                wxString sFourCC = Config::ReadString(Config::sPathVideoOverruleFourCC);
                wxCharBuffer chars = sFourCC.mb_str();
                int fourcc = 0;
                switch (chars.length())
                {
                case 4: fourcc += chars[static_cast<size_t>(3)] << 24; // Fallthrough
                case 3: fourcc += chars[static_cast<size_t>(2)] << 16; // Fallthrough
                case 2: fourcc += chars[static_cast<size_t>(1)] << 8;  // Fallthrough
                case 1: fourcc += chars[static_cast<size_t>(0)];        break;
                default: FATAL("FourCC length must be 1, 2, 3, or 4.");
                }
                videoCodec->codec_tag = fourcc;
            }
            VAR_INFO(videoCodec);
        }

        if (storeAudio)
        {
            audioStream = outputformat->getAudioCodec()->addStream(context);
            audioCodec = audioStream->codec;
            VAR_INFO(audioCodec);
        }

        av_dump_format(context, 0, filename.c_str(), 1);

        //////////////////////////////////////////////////////////////////////////
        // OPEN CODECS AND ALLOCATE BUFFERS
        //////////////////////////////////////////////////////////////////////////

        if (storeVideo)
        {
            if (!outputformat->getVideoCodec()->open(videoCodec))
            {
                throw EncodingError(_("Failed to open video codec."));
            }

            outputPicture = alloc_picture(videoCodec->pix_fmt, videoCodec->width, videoCodec->height);
            ASSERT(outputPicture);

            // if the output format is not RGB24P, then a temporary picture is needed too. It is then converted to the required output format
            if (videoCodec->pix_fmt != AV_PIX_FMT_RGB24)
            {
                colorSpaceConversionPicture = alloc_picture(AV_PIX_FMT_RGB24, videoCodec->width, videoCodec->height);
                ASSERT(colorSpaceConversionPicture);

                static int sws_flags = SWS_BICUBIC;
                colorSpaceConversionContext = sws_getCachedContext(colorSpaceConversionContext, videoCodec->width, videoCodec->height, AV_PIX_FMT_RGB24, videoCodec->width, videoCodec->height, videoCodec->pix_fmt, sws_flags, 0, 0, 0);
                ASSERT_NONZERO(colorSpaceConversionContext);
            }

            videoParameters->setBoundingBox(wxSize(videoCodec->width, videoCodec->height)).setDrawBoundingBox(false).setOptimizeForQuality();

            videoOpened = true;
        }

        if (storeAudio)
        {
            if (!outputformat->getAudioCodec()->open(audioCodec))
            {
                throw EncodingError(_("Failed to open audio codec."));
            }

            nRequiredInputSamplesPerChannel = audioCodec->frame_size;
            if (audioCodec->frame_size <= 1)
            {
                // ugly hack for PCM codecs (will be removed ASAP with new PCM support to compute the input frame size in samples
                // See libavformat/output-example.c
                nRequiredInputSamplesPerChannel = 10000;
                switch (audioCodec->codec_id)
                {
                case AV_CODEC_ID_PCM_S16LE:
                case AV_CODEC_ID_PCM_S16BE:
                case AV_CODEC_ID_PCM_U16LE:
                case AV_CODEC_ID_PCM_U16BE:
                    nRequiredInputSamplesPerChannel >>= 1;
                    break;
                default:
                    break;
                }
            }

            nRequiredInputSamplesForAllChannels = nRequiredInputSamplesPerChannel * audioCodec->channels;
            samples = static_cast<sample*>(av_malloc(Convert::audioSamplesToBytes(nRequiredInputSamplesForAllChannels)));

            if (audioCodec->sample_fmt != AV_SAMPLE_FMT_S16)
            {
                bytesPerSampleForEncoder = av_get_bytes_per_sample(audioCodec->sample_fmt);
                if (bytesPerSampleForEncoder == 0)
                {
                    VAR_ERROR(audioCodec->sample_fmt)(audioCodec)(*this);
                    throw EncodingError(_("Failed to determine audio sample size for encoding."));
                }

                audioSampleFormatResampleContext = swr_alloc_set_opts(0,
                                                                      audioCodec->channel_layout, audioCodec->sample_fmt, audioCodec->sample_rate,
                                                                      audioCodec->channel_layout, AV_SAMPLE_FMT_S16, audioCodec->sample_rate, 0, 0);
                ASSERT_NONZERO(audioSampleFormatResampleContext);

                int result = swr_init(audioSampleFormatResampleContext);
                if (result < 0)
                {
                    VAR_ERROR(result)(avcodecErrorString(result));
                    throw EncodingError(_("Failed to initialize audio resampler"));
                }

                nAudioPlanes = av_sample_fmt_is_planar(audioCodec->sample_fmt) ? audioCodec->channels : 1;
                resampledAudioData = new uint8_t*[nAudioPlanes];
                result = av_samples_alloc(resampledAudioData, 0, audioCodec->channels, nRequiredInputSamplesPerChannel, audioCodec->sample_fmt, 0);
                if (result < 0)
                {
                    VAR_ERROR(result)(avcodecErrorString(result));
                    throw EncodingError(_("Failed to allocate memory for resampling"));
                }


            }

            audioParameters->setSampleRate(audioCodec->sample_rate).setNrChannels(audioCodec->channels);

            audioOpened = true;
        }

        //////////////////////////////////////////////////////////////////////////
        // OPEN FILE
        //////////////////////////////////////////////////////////////////////////

        ASSERT(!(context->flags & AVFMT_NOFILE))(context);
        if (avio_open(&context->pb, filename.c_str(), AVIO_FLAG_WRITE) < 0)
        {
            VAR_ERROR(filename);
            throw EncodingError(_("Failed to open file for writing."));
        }

        fileOpened = true;

        avformat_write_header(context, 0);

        //////////////////////////////////////////////////////////////////////////
        // WRITE DATA INTO THE FILE
        //////////////////////////////////////////////////////////////////////////

        AudioChunkPtr currentAudioChunk = storeAudio ? sequence->getNextAudio(*audioParameters) : AudioChunkPtr();

        while (!isAborted())  // write interleaved audio and video frames
        {
            // todo remove use of pts (deprecated)
            double audioTime = storeAudio ? ((double)audioStream->pts.val) *  (double)audioStream->time_base.num / (double)audioStream->time_base.den : lengthInSeconds;
            double videoTime = storeVideo ? ((double)videoStream->pts.val) * (double)videoStream->time_base.num / (double)videoStream->time_base.den : lengthInSeconds;

            bool writeAudio = storeAudio && audioTime < (double)lengthInSeconds;
            bool writeVideo = storeVideo && videoTime < (double)lengthInSeconds;

            if (!writeAudio && !writeVideo)
            {
                break; // Done.
            }

            //////////////////////////////////////////////////////////////////////////
            // SHOW PROGRESS
            //////////////////////////////////////////////////////////////////////////

            wxString s; s << _("(frame ") << (position - mFrom) << _(" out of ") << mLength << ")";
            showProgressText(ps + " " + s);
            showProgress(position - mFrom);

            if (writeAudio && audioTime < videoTime)
            {
                //////////////////////////////////////////////////////////////////////////
                // AUDIO
                //////////////////////////////////////////////////////////////////////////

                sample* q = samples;
                samplecount remainingSamples = nRequiredInputSamplesForAllChannels;
                AVFrame* encodeFrame = 0;

                if (currentAudioChunk)
                {
                    //////////////////////////////////////////////////////////////////////////
                    // EXTRACT AUDIO SAMPLES FROM INPUT
                    //////////////////////////////////////////////////////////////////////////

                    while (remainingSamples > 0)
                    {
                        if (currentAudioChunk)
                        {
                            // Previous chunk not used completely
                            samplecount nSamples = currentAudioChunk->extract(q, remainingSamples);
                            q += nSamples;
                            remainingSamples -= nSamples;
                            ASSERT_MORE_THAN_EQUALS_ZERO(remainingSamples);

                            if (currentAudioChunk->getUnreadSampleCount() == 0)
                            {
                                currentAudioChunk = sequence->getNextAudio(*audioParameters);
                                if (currentAudioChunk &&
                                    currentAudioChunk->getPts() > position &&
                                    (currentAudioChunk->getPts() - mFrom) < mLength) // Avoid showing progress 48 out of 47 frames
                                {
                                    position = currentAudioChunk->getPts();
                                }
                            }
                        }
                        else // Generate silence to fill last input packet
                        {
                            while (remainingSamples-- > 0) { *q++ = 0; }
                        }
                    }

                    //////////////////////////////////////////////////////////////////////////
                    // CONVERT AUDIO SAMPLES TO INPUT FORMAT REQUIRED FOR ENCODER
                    //////////////////////////////////////////////////////////////////////////

                    encodeFrame = new AVFrame;
                    encodeFrame->data[0] = (uint8_t*)samples;
                    encodeFrame->linesize[0] = Convert::audioSamplesToBytes(nRequiredInputSamplesForAllChannels);
                    if (audioSampleFormatResampleContext != 0)
                    {
                        int nOutputFrames = swr_convert(
                            audioSampleFormatResampleContext, resampledAudioData, nRequiredInputSamplesPerChannel,
                            const_cast<const uint8_t**>(encodeFrame->data), nRequiredInputSamplesPerChannel);
                        ASSERT_EQUALS(nOutputFrames, nRequiredInputSamplesPerChannel);

                        delete encodeFrame;
                        encodeFrame = new AVFrame();

                        for (int i = 0; i < nAudioPlanes; ++i)
                        {
                            encodeFrame->data[i] = resampledAudioData[i];
                            encodeFrame->linesize[i] = nRequiredInputSamplesPerChannel * bytesPerSampleForEncoder;
                        }
                    }
                }
                // else: all input fed into encoder

                //////////////////////////////////////////////////////////////////////////
                // ENCODE AUDIO INTO AN OUTPUT PACKET
                //////////////////////////////////////////////////////////////////////////

                AVPacket* audioPacket = new AVPacket();
                audioPacket->data = 0;
                audioPacket->size = 0;
                int gotPacket = 0;

                // CODEC_CAP_DELAY (see declaration of avcodec_encode_audio2) is not used: extra silence is added at the end?
                if (encodeFrame != 0)
                {
                    encodeFrame->format = audioCodec->sample_fmt;
                    encodeFrame->channel_layout = audioCodec->channel_layout;
                    encodeFrame->channels = audioCodec->channels;
                    encodeFrame->sample_rate = audioCodec->sample_rate;
                    encodeFrame->nb_samples = nRequiredInputSamplesPerChannel;
                    encodeFrame->pts = av_rescale_q(fedSamples, sampleTimeBase, audioCodec->time_base);
                    fedSamples += nRequiredInputSamplesPerChannel;
                }

                int result = avcodec_encode_audio2(audioCodec, audioPacket, encodeFrame, &gotPacket); // if gotPacket == 0, then packet is destructed
                if (0 != result)
                {
                    VAR_ERROR(result)(avcodecErrorString(result))(*this);
                    throw EncodingError(_("Failed to encode audio data."));
                }

                delete encodeFrame; // May be 0

                //////////////////////////////////////////////////////////////////////////
                // WRITE AUDIO INTO FILE
                //////////////////////////////////////////////////////////////////////////

                if (gotPacket)
                {
                    audioPacket->flags |= AV_PKT_FLAG_KEY;
                    audioPacket->stream_index = audioStream->index;
                    av_packet_rescale_ts(audioPacket, audioCodec->time_base, audioStream->time_base);
                    int result = av_interleaved_write_frame(context, audioPacket); // av_interleaved_write_frame: transfers ownership of packet
                    if (0 != result)
                    {
                        VAR_ERROR(result)(avcodecErrorString(result))(*this);
                        throw EncodingError(_("Failed to write audio data."));
                    }
                }
                // else Packet possibly buffered inside codec
                delete audioPacket;
            }
            else if (writeVideo)
            {
                //////////////////////////////////////////////////////////////////////////
                // VIDEO
                //////////////////////////////////////////////////////////////////////////

                int64_t videoPacketPts = AV_NOPTS_VALUE;
                AVFrame* toBeEncodedPicture = 0;

                if (!videoEnd)
                {
                    VideoFramePtr frame = sequence->getNextVideo(*videoParameters);
                    if (!frame)
                    {
                        videoEnd = true;
                    }
                    else
                    {
                        if (frame->getPts() > position &&
                            (frame->getPts() - mFrom) < mLength) // Avoid showing progress 48 out of 47 frames
                        {
                            position = frame->getPts();
                        }

                        videoPacketPts = frame->getPts() - mFrom;
                        if (frame->getForceKeyFrame())
                        {
                            outputPicture->key_frame = 1;
                            outputPicture->pict_type = AV_PICTURE_TYPE_I;
                        }
                        else
                        {
                            outputPicture->key_frame = 0;
                            outputPicture->pict_type = AV_PICTURE_TYPE_NONE;
                        }
                        wxImagePtr image = frame->isA<model::EmptyFrame>() ? wxImagePtr() : frame->getImage(); // Performance optimization for empty frames (do not create useless 0 data).

                        //////////////////////////////////////////////////////////////////////////
                        // CONVERT VIDEO TO REQUIRED FORMAT FOR ENCODER
                        //////////////////////////////////////////////////////////////////////////

                        int rgbImageSize = avpicture_get_size(AV_PIX_FMT_RGB24, videoCodec->width, videoCodec->height);
                        AVFrame* toBeFilledPicture = (colorSpaceConversionContext == 0) ? outputPicture : colorSpaceConversionPicture;
                        if (!image)
                        {
                            memset(toBeFilledPicture->data[0], 0, rgbImageSize); // Empty. Fill with 0.
                        }
                        else
                        {
                            memcpy(toBeFilledPicture->data[0], image->GetData(), rgbImageSize);
                        }
                        if (colorSpaceConversionContext != 0)
                        {
                            sws_scale(colorSpaceConversionContext, colorSpaceConversionPicture->data, colorSpaceConversionPicture->linesize, 0, videoCodec->height, outputPicture->data, outputPicture->linesize);
                        }
                        outputPicture->pts = videoPacketPts;
                        toBeEncodedPicture = outputPicture;
                    }
                }
                // else (videoEnd): no more frames to compress. The codec has a latency of a few frames if using B frames, so we get the last frames by passing 0.

                AVPacket* videoPacket = new AVPacket();
                videoPacket->stream_index = videoStream->index;
                videoPacket->data = 0;
                videoPacket->size = 0;
                videoPacket->duration = 1;
                if (context->oformat->flags & AVFMT_RAWPICTURE)
                {
                    //////////////////////////////////////////////////////////////////////////
                    // WRITE RAW VIDEO
                    //////////////////////////////////////////////////////////////////////////

                    // raw video case. The API will change slightly in the near future for that
                    ASSERT_NONZERO(toBeEncodedPicture);
                    av_init_packet(videoPacket);
                    videoPacket->flags |= AV_PKT_FLAG_KEY;
                    videoPacket->data = (uint8_t *)toBeEncodedPicture;
                    videoPacket->size = sizeof(AVPicture);
                    videoPacket->pts = toBeEncodedPicture->pts;
                    videoPacket->dts = videoPacket->pts;
                    int result = av_interleaved_write_frame(context, videoPacket); // av_interleaved_write_frame: transfers ownership of packet
                    if (0 != result)
                    {
                        VAR_ERROR(result)(avcodecErrorString(result))(*this);
                        throw EncodingError(_("Failed to write raw video data."));
                    }
                }
                else
                {
                    //////////////////////////////////////////////////////////////////////////
                    // ENCODE VIDEO
                    //////////////////////////////////////////////////////////////////////////

                    int gotPacket = 0;
                    int result = avcodec_encode_video2(videoCodec, videoPacket, toBeEncodedPicture, &gotPacket);  // if gotPacket == 0, then packet is destructed
                    if (0 != result)
                    {
                        VAR_ERROR(result)(avcodecErrorString(result))(*this);
                        throw EncodingError(_("Failed to encode video data."));
                    }

                    //////////////////////////////////////////////////////////////////////////
                    // WRITE ENCODED VIDEO
                    //////////////////////////////////////////////////////////////////////////

                    if (gotPacket)
                    {
                        av_packet_rescale_ts(videoPacket, videoCodec->time_base, videoStream->time_base);
                        int result = av_interleaved_write_frame(context, videoPacket); // av_interleaved_write_frame: transfers ownership of packet
                        if (0 != result)
                        {
                            VAR_ERROR(result)(avcodecErrorString(result))(*this);
                            throw EncodingError(_("Failed to write video data."));
                        }
                    }
                    // else Packet possibly buffered inside codec
                }
                delete videoPacket;
            }
        }

        int result = av_write_trailer(context);
        if (result != 0)
        {
            VAR_ERROR(result)(avcodecErrorString(result))(*this);
            throw EncodingError(_("Failed to close file."));
        }
    }
    catch (EncodingError error)
    {
        VAR_ERROR(error);
        gui::Dialog::get().getConfirmation(_("Rendering failed"), error.message);
    }

    //////////////////////////////////////////////////////////////////////////
    // CLOSE CODECS AND BUFFERS
    //////////////////////////////////////////////////////////////////////////

    if (videoOpened)
    {
        avcodec_close(videoCodec);
        av_freep(&outputPicture->data[0]);
        av_frame_free(&outputPicture);
        if (colorSpaceConversionPicture)
        {
            av_freep(&colorSpaceConversionPicture->data[0]);
            av_frame_free(&colorSpaceConversionPicture);
        }
    }

    if (audioOpened)
    {
        if (audioSampleFormatResampleContext != 0)
        {
            av_freep(&resampledAudioData[0]);
            delete[] resampledAudioData;
            swr_free(&audioSampleFormatResampleContext);
        }
        avcodec_close(audioCodec);
        av_freep(&samples);
    }

    //////////////////////////////////////////////////////////////////////////
    // CLOSE STREAMS
    //////////////////////////////////////////////////////////////////////////

    for (unsigned int i = 0; i < context->nb_streams; i++)
    {
        av_freep(&context->streams[i]->codec);
        av_freep(&context->streams[i]);
    }

    ASSERT(!(context->flags & AVFMT_NOFILE))(context);

    //////////////////////////////////////////////////////////////////////////
    // CLOSE FILE
    //////////////////////////////////////////////////////////////////////////

    if (fileOpened)
    {
        avio_close(context->pb);
    }

    av_freep(&context);
}

static AVFrame *alloc_picture(const enum PixelFormat& pix_fmt, int width, int height)
{
    AVFrame* picture = av_frame_alloc();
    ASSERT(picture);

    int size = avpicture_get_size(pix_fmt, width, height);
    uint8_t* picture_buf = (uint8_t*)av_malloc(size);
    ASSERT(picture_buf);
    avpicture_fill((AVPicture *)picture, picture_buf, pix_fmt, width, height);
    return picture;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const Render& obj)
{
    os  << &obj               << '|'
        << obj.mFileName      << '|'
        << *obj.mOutputFormat << '|'
        << obj.mSeparateAtCuts;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

const std::string sFileName("filename");

template<class Archive>
void Render::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        if (Archive::is_loading::value)
        {
            if (version < 2) // Loading old version.
            {
                ar & BOOST_SERIALIZATION_NVP(mFileName);
            }
            else
            {
                wxString path; // Stored as string, to enable storing absolute path.
                ar & boost::serialization::make_nvp(sFileName.c_str(), path);
                mFileName = util::path::fromSaveString(path);
            }
        }
        else
        {
            wxFileName path{ mFileName };
            path.MakeAbsolute();
            wxString saveString{ util::path::toSaveString(path) };
            if (!mFileName.HasExt() && !mFileName.HasName())
            {
                // Empty (default) filename is saved as empty string (platform independence for test save files).
                saveString = "";
            }
            ar & boost::serialization::make_nvp(sFileName.c_str(), saveString);
        }
        ar & BOOST_SERIALIZATION_NVP(mOutputFormat);
        ar & BOOST_SERIALIZATION_NVP(mSeparateAtCuts);
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

template void Render::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void Render::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

}} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::render::Render)
