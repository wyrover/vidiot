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

#include "Render.h"

#include "AudioChunk.h"
#include "AudioCodec.h"
#include "AudioCodecs.h"
#include "AudioCompositionParameters.h"
#include "Config.h"
#include "Constants.h"
#include "Convert.h"
#include "Dialog.h"
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
#include "UtilSerializeBoost.h"
#include "UtilSerializeWxwidgets.h"
#include "VideoCodec.h"
#include "VideoCodecs.h"
#include "VideoCompositionParameters.h"
#include "VideoFrame.h"
#include "Work.h"
#include "Worker.h"

namespace model { namespace render {

static AVFrame *alloc_picture(const enum PixelFormat& pix_fmt, const int& width, const int& height);

//////////////////////////////////////////////////////////////////////////
// RENDERING
//////////////////////////////////////////////////////////////////////////

class RenderWork : public worker::Work
{
public:
    RenderWork(const SequencePtr& sequence, const pts& from, const pts& to)
    :   worker::Work(boost::bind(&RenderWork::generate,this))
    ,   mSequence(sequence)
    ,   mFrom(from)
    ,   mTo(to)
    ,   mMaxLength(Convert::timeToPts(Config::ReadLong(Config::sPathDebugMaxRenderLength) *  Constants::sSecond))
    {
    }

    void generate();

private:

    model::SequencePtr mSequence;
    pts mFrom;
    pts mTo;
    pts mMaxLength;
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
    ,   mFileName()
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
    if (!mFileName.IsOk()) { return false; }
    if (mFileName.IsDir()) { return false; }
    if (!mFileName.HasExt()) { return false; }
    if (!mFileName.HasName()) { return false; }
    if (mFileName.FileExists() && !mFileName.IsFileWritable()) { return false; }
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
        worker::Worker::get().schedule(boost::make_shared<RenderWork>(clone,0,clone->getLength()));
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
                worker::Worker::get().schedule(boost::make_shared<RenderWork>(clone,start,end));
                ++c;
            }
            // else: If there's only empty space at this interval's begin point: do not render the empty space and skip to the next interval.
            ++it;
            start = end;
        }
    }
}

typedef std::list<SequencePtr> Sequences;
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
    std::list<wxString> errors;

    std::list<wxFileName> allFilenames;
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

void RenderWork::generate()
{
    setThreadName("RenderWork::generate");
    model::SequencePtr sequence = mSequence;
    pts from = mFrom;
    pts to = mTo;
    RenderPtr mRender = sequence->getRender();

    VAR_INFO(sequence)(from)(to);
    ASSERT_MORE_THAN_EQUALS_ZERO(from);
    ASSERT_MORE_THAN_ZERO(to);
    ASSERT_LESS_THAN_EQUALS(from,sequence->getLength());
    ASSERT_LESS_THAN_EQUALS(to,sequence->getLength());
    sequence->moveTo(from);
    int length = to - from;
    showProgressBar(length);
    wxString ps; ps << _("Rendering sequence '") << sequence->getName() << "'";
    showProgressText(ps);

    OutputFormatPtr outputformat = mRender->getOutputFormat();
    AVFormatContext* context = outputformat->getContext();
    ASSERT(mRender->getFileName().IsOk());
    ASSERT_NONZERO(context);
    wxString filename = mRender->getFileName().GetFullPath();
    ASSERT_LESS_THAN_EQUALS(sizeof(filename.c_str()),sizeof(context->filename));
    _snprintf(context->filename, sizeof(context->filename), "%s", filename.c_str());

    bool storeAudio = outputformat->storeAudio();
    bool storeVideo = outputformat->storeVideo();
    ASSERT(storeAudio || storeVideo)(storeAudio)(storeVideo);

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

    uint16_t* samples = 0;
    samplecount audioEncodeRequiredInputSize = 0;
    SwrContext* audioSampleFormatResampleContext = 0;

    bool ok = true;
    wxString sErrorMessage;

    if (ok && storeVideo)
    {
        videoStream = outputformat->getVideoCodec()->addStream(context);
        videoCodec = videoStream->codec;
        if (Config::Exists(Config::sPathOverruleFourCC))
        {
            wxString sFourCC = Config::ReadString(Config::sPathOverruleFourCC);
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
    if (ok && storeAudio)
    {
        audioStream = outputformat->getAudioCodec()->addStream(context);
        audioCodec = audioStream->codec;
        VAR_INFO(audioCodec);
    }

    av_dump_format(context, 0, filename.c_str(), 1);

    // now that all the parameters are set, we can open the audio and video codecs and allocate the necessary encode buffers
    if (ok && storeVideo)
    {
        ok = outputformat->getVideoCodec()->open(videoCodec);

        if (ok)
        {
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
            videoOpened = true;
        }
        else
        {
            sErrorMessage << "Failed to open video codec.";
        }
    }

    if (ok && storeAudio)
    {
        ok = outputformat->getAudioCodec()->open(audioCodec);

        if (ok)
        {
            ASSERT_NONZERO(audioCodec->frame_size);
            // ugly hack for PCM codecs (will be removed ASAP with new PCM support to compute the input frame size in samples
            // See libavformat/output-example.c
            if (audioCodec->frame_size <= 1)
            {
                audioEncodeRequiredInputSize = 10000;
                switch (audioCodec->codec_id)
                {
                case CODEC_ID_PCM_S16LE:
                case CODEC_ID_PCM_S16BE:
                case CODEC_ID_PCM_U16LE:
                case CODEC_ID_PCM_U16BE:
                    audioEncodeRequiredInputSize >>= 1;
                    break;
                default:
                    break;
                }
            }
            else
            {
                audioEncodeRequiredInputSize = Convert::audioFramesToSamples(audioCodec->frame_size,audioCodec->channels);
            }
            samples = static_cast<uint16_t*>(av_malloc(Convert::audioSamplesToBytes(audioEncodeRequiredInputSize)));

             if (audioCodec->sample_fmt != AV_SAMPLE_FMT_S16)
             {
                audioSampleFormatResampleContext = swr_alloc_set_opts(0,
                    audioCodec->channel_layout, audioCodec->sample_fmt, audioCodec->sample_rate,
                    audioCodec->channel_layout, AV_SAMPLE_FMT_S16, audioCodec->sample_rate, 0, 0);
                ASSERT_NONZERO(audioSampleFormatResampleContext);

                int result = swr_init(audioSampleFormatResampleContext);
                ASSERT_ZERO(result)(avcodecErrorString(result));
             }

            audioOpened = true;
        }
        else
        {
            sErrorMessage << "Failed to open audio codec.";
        }
    }

    VideoCompositionParameters videoparameters = VideoCompositionParameters().setBoundingBox(wxSize(videoCodec->width,videoCodec->height)).setDrawBoundingBox(false).setOptimizeForQuality();
    AudioCompositionParameters audioparameters = AudioCompositionParameters().setSampleRate(audioCodec->sample_rate).setNrChannels(audioCodec->channels); // todo pass required pixel format also?

    if (ok)
    {
        // Write the actual data into the file

        // Open the output file
        ASSERT(!(context->flags & AVFMT_NOFILE))(context);
        int result = avio_open(&context->pb, filename.c_str(), AVIO_FLAG_WRITE);
        ok = result >= 0;
        if (!ok)
        {
            VAR_ERROR(filename)(result)(avcodecErrorString(result))(*this);
            sErrorMessage << "Failed to open file '" << filename << "' for writing.";
        }
        else
        {
            fileOpened = true;

            ASSERT_MORE_THAN_EQUALS_ZERO(result)(filename)(avcodecErrorString(result))(*this);

            avformat_write_header(context,0);

            pts numberOfEncodecInputAudioFrames = 0;
            pts numberOfWrittenOutputAudioFrames = 0;
            pts numberOfReadInputVideoFrames = 0;
            pts numberOfWrittenOutputVideoFrames = 0;

            pts lengthInVideoFrames = length;
            if ((mMaxLength > 0) && (lengthInVideoFrames > mMaxLength))
            {
                lengthInVideoFrames = mMaxLength;
            }
            long lengthInSeconds = Convert::ptsToTime(lengthInVideoFrames) / Constants::sSecond;

            AudioChunkPtr currentAudioChunk = sequence->getNextAudio(audioparameters);

            double audioTime = storeAudio ? 0.0 : std::numeric_limits<double>::max();
            double videoTime = storeVideo ? 0.0 : std::numeric_limits<double>::max();

            while (!isAborted())  // write interleaved audio and video frames
            {
                if (storeAudio) { audioTime = (double)audioStream->pts.val * (double)audioStream->time_base.num / (double)audioStream->time_base.den; }
                if (storeVideo) { videoTime = (double)videoStream->pts.val * (double)videoStream->time_base.num / (double)videoStream->time_base.den; }

                if ((!storeAudio || audioTime >= lengthInSeconds) &&
                    (!storeVideo || videoTime >= lengthInSeconds))
                {
                    break;
                }

                if (storeAudio && audioTime < videoTime) // Write audio frame
                {
                    uint16_t* q = samples;
                    samplecount remainingSamples = audioEncodeRequiredInputSize;

                    while (remainingSamples > 0)
                    {
                        if (currentAudioChunk)
                        {
                            // Previous chunk not used completely
                            samplecount nSamples = min(remainingSamples, currentAudioChunk->getUnreadSampleCount());
                            currentAudioChunk->extract(q,nSamples);
                            q += nSamples;
                            remainingSamples -= nSamples;

                            if (currentAudioChunk->getUnreadSampleCount() == 0)
                            {
                                currentAudioChunk = sequence->getNextAudio(audioparameters);
                            }
                        }
                        else // Generate silence to fill last input packet
                        {
                            while (remainingSamples-- > 0) { *q++ = 0; }
                        }
                    }

                    int nPlanes = av_sample_fmt_is_planar(audioCodec->sample_fmt) ? audioCodec->channels : 1;
                    AVFrame* encodeFrame = new AVFrame;
                    encodeFrame->data[0] = (uint8_t*)samples;
                    encodeFrame->linesize[0] = audioEncodeRequiredInputSize * AudioChunk::sBytesPerSample;
                    if (audioSampleFormatResampleContext != 0)
                    {
                        uint8_t** data = new uint8_t*[nPlanes];

                        int result = av_samples_alloc(data,0,audioCodec->channels,audioCodec->frame_size,audioCodec->sample_fmt,0);
                        int nOutputFrames = swr_convert(
                            audioSampleFormatResampleContext, data, audioCodec->frame_size,
                            const_cast<const uint8_t**>(encodeFrame->data), audioCodec->frame_size);
                        ASSERT_MORE_THAN_ZERO(nOutputFrames);
                        ASSERT_EQUALS(nOutputFrames,audioCodec->frame_size);

                        delete encodeFrame;
                        encodeFrame = new AVFrame;

                        for (int i = 0; i < nPlanes; ++i)
                        {
                            encodeFrame->data[i] = data[i];
                            encodeFrame->linesize[i] = audioEncodeRequiredInputSize * AudioChunk::sBytesPerSample;
                        }
                    }

                    encodeFrame->nb_samples = audioCodec->frame_size;
                    encodeFrame->pts = AV_NOPTS_VALUE; // NOT: numberOfEncodecInputAudioFrames * audioCodec->frame_size; - causes silence...

                    AVPacket* audioPacket = new AVPacket();
                    audioPacket->data = 0;
                    audioPacket->size = 0;
                    int gotPacket = 0;

                    // CODEC_CAP_DELAY (see declaration  of avcodec_encode_audio2) is not used: extra silence is added at the end?
                    int result = avcodec_encode_audio2(audioCodec, audioPacket, encodeFrame, &gotPacket); // if gotPacket == 0, then packet is destructed
                    ASSERT_ZERO(result)(avcodecErrorString(result))(*this);
                    numberOfEncodecInputAudioFrames++;

                    if (audioSampleFormatResampleContext)
                    {
                        av_freep(&encodeFrame->data[0]);
                    }
                    delete encodeFrame;

                    if (gotPacket)
                    {
                        audioPacket->flags |= AV_PKT_FLAG_KEY;
                        audioPacket->stream_index = audioStream->index;
                        int result = av_interleaved_write_frame(context, audioPacket); // av_interleaved_write_frame: transfers ownership of packet
                        ASSERT_ZERO(result)(avcodecErrorString(result))(*this);
                        numberOfWrittenOutputAudioFrames++;
                    }
                    // else Packet possibly buffered inside codec
                }
                else
                {
                    // Write video frame
                    AVFrame* toBeEncodedPicture = 0;
                    if (!videoEnd)
                    {
                        VideoFramePtr frame = sequence->getNextVideo(videoparameters);
                        if (!frame)
                        {
                            videoEnd = true;
                        }
                        else
                        {
                            numberOfReadInputVideoFrames++;
                            if (frame->getForceKeyFrame())
                            {
                                outputPicture->key_frame = 1;
                                outputPicture->pict_type =  AV_PICTURE_TYPE_I;
                            }
                            else
                            {
                                outputPicture->key_frame = 0;
                                outputPicture->pict_type =  AV_PICTURE_TYPE_NONE;
                            }
                            pts progress = frame->getPts() - from;
                            wxString s; s << _("(frame ") << progress << _(" out of ") << length << ")";
                            showProgressText(ps + " " + s);
                            showProgress(progress);
                            wxImagePtr image = frame->getImage();

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
                            outputPicture->pts = numberOfReadInputVideoFrames;
                            toBeEncodedPicture = outputPicture;
                        }
                    }
                    // else (videoEnd): no more frames to compress. The codec has a latency of a few frames if using B frames, so we get the last frames by passing 0.

                    if (context->oformat->flags & AVFMT_RAWPICTURE)
                    {
                        // raw video case. The API will change slightly in the near future for that
                        AVPacket* videoPacket = new AVPacket();
                        av_init_packet(videoPacket);
                        videoPacket->flags |= AV_PKT_FLAG_KEY;
                        videoPacket->stream_index= videoStream->index;
                        videoPacket->data = (uint8_t *)toBeEncodedPicture;
                        videoPacket->size = sizeof(AVPicture);
                        videoPacket->pts = numberOfWrittenOutputVideoFrames;
                        int result = av_interleaved_write_frame(context, videoPacket); // av_interleaved_write_frame: transfers ownership of packet
                        ASSERT_ZERO(result)(avcodecErrorString(result))(*this);
                        numberOfWrittenOutputVideoFrames++;
                    }
                    else
                    {
                        // encode the image
                        int gotPacket = 0;
                        AVPacket* videoPacket = new AVPacket();
                        videoPacket->data = 0;
                        videoPacket->size = 0;
                        VAR_DEBUG(numberOfReadInputVideoFrames)(numberOfWrittenOutputVideoFrames)(videoCodec);
                        result = avcodec_encode_video2(videoCodec, videoPacket, toBeEncodedPicture, &gotPacket);  // if gotPacket == 0, then packet is destructed
                        ASSERT_ZERO(result)(avcodecErrorString(result))(*this);
                        if (gotPacket)
                        {
                            int result = av_interleaved_write_frame(context, videoPacket); // av_interleaved_write_frame: transfers ownership of packet
                            ASSERT_ZERO(result)(avcodecErrorString(result))(*this);
                            numberOfWrittenOutputVideoFrames++;
                        }
                        // else Packet possibly buffered inside codec
                    }
                }
            }

            int ret = av_write_trailer(context);
            ASSERT_ZERO(ret)(avcodecErrorString(ret))(*this);
        }
    }

    if (!ok)
    {
        gui::Dialog::get().getConfirmation(_("Rendering failed"), sErrorMessage);
    }

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
            swr_free(&audioSampleFormatResampleContext);
        }
        avcodec_close(audioCodec);
        av_freep(&samples);
    }

    for (unsigned int i = 0; i < context->nb_streams; i++)
    {
        av_freep(&context->streams[i]->codec);
        av_freep(&context->streams[i]);
    }

    ASSERT(!(context->flags & AVFMT_NOFILE))(context);

    if (fileOpened)
    {
        avio_close(context->pb);
    }

    av_freep(&context);
}

static AVFrame *alloc_picture(const enum PixelFormat& pix_fmt, const int& width, const int& height)
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

template<class Archive>
void Render::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_NVP(mFileName);
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