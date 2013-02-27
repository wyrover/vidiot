#include "Render.h"

#include "AudioChunk.h"
#include "AudioCodec.h"
#include "AudioCodecs.h"
#include "AudioCompositionParameters.h"
#include "Config.h"
#include "Constants.h"
#include "Convert.h"
#include "Folder.h"
#include "OutputFormat.h"
#include "OutputFormats.h"
#include "Project.h"
#include "Properties.h"
#include "Sequence.h"
#include "StatusBar.h"
#include "UtilLog.h"
#include "UtilLogAvcodec.h"
#include "UtilLogWxwidgets.h"
#include "UtilSerializeBoost.h"
#include "UtilSerializeWxwidgets.h"
#include "VideoCodec.h"
#include "VideoCodecs.h"
#include "VideoCompositionParameters.h"
#include "VideoFrame.h"
#include "Worker.h"

namespace model { namespace render {

static AVFrame *alloc_picture(enum PixelFormat pix_fmt, int width, int height);

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

void Render::setOutputFormat(OutputFormatPtr format)
{
    mOutputFormat = format;
}

wxFileName Render::getFileName() const
{
    return mFileName;
}

void Render::setFileName(wxFileName filename)
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
void Render::schedule(SequencePtr sequence)
{
    if (!sequence->getRender()->getSeparateAtCuts())
    {
        model::SequencePtr clone = make_cloned<model::Sequence>(sequence);
        gui::Worker::get().schedule(boost::make_shared<Work>(boost::bind(&Render::generate,clone->getRender(),clone,0,clone->getLength())));
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
            model::SequencePtr clone = make_cloned<model::Sequence>(sequence);
            wxFileName fn = clone->getRender()->getFileName();
            fn.SetName(fn.GetName() + wxString::Format("_%d",c));
            clone->getRender()->setFileName(fn);
            gui::Worker::get().schedule(boost::make_shared<Work>(boost::bind(&Render::generate,clone->getRender(),clone,start,end)));
            ++it;
            ++c;
            start = end;
        }
    }
}

typedef std::list<SequencePtr> Sequences;
void findSequences(FolderPtr node, Sequences& result)
{
    BOOST_FOREACH( NodePtr child, node->getChildren() )
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
    BOOST_FOREACH( SequencePtr sequence, seqs )
    {
        allFilenames.push_back(sequence->getRender()->getFileName());
    }

    BOOST_FOREACH( SequencePtr sequence, seqs )
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
        wxMessageBox(error, _("Not all sequences have been scheduled:"),wxOK);
    }
}

//////////////////////////////////////////////////////////////////////////
// RENDERING
//////////////////////////////////////////////////////////////////////////

void Render::generate(model::SequencePtr sequence, pts from, pts to)
{
    VAR_INFO(sequence)(from)(to);
    ASSERT_MORE_THAN_EQUALS_ZERO(from);
    ASSERT_MORE_THAN_ZERO(to);
    ASSERT_LESS_THAN_EQUALS(from,sequence->getLength());
    ASSERT_LESS_THAN_EQUALS(to,sequence->getLength());
    sequence->moveTo(from);
    int length = to - from;
    gui::StatusBar::get().showProgressBar(length);
    wxString ps; ps << _("Rendering sequence '") << sequence->getName() << "'";
    gui::StatusBar::get().setProcessingText(ps);

    AVFormatContext* context = mOutputFormat->getContext();
    ASSERT(mFileName.IsOk());
    wxString filename = mFileName.GetFullPath();
    ASSERT_LESS_THAN_EQUALS(sizeof(filename.c_str()),sizeof(context->filename));
    _snprintf(context->filename, sizeof(context->filename), "%s", filename.c_str());

    bool storeAudio = mOutputFormat->storeAudio();
    bool storeVideo = mOutputFormat->storeVideo();
    ASSERT(storeAudio || storeVideo)(storeAudio)(storeVideo);

    bool videoOpened = false;
    bool audioOpened = false;

    AVStream* videoStream = 0;
    AVStream* audioStream = 0;

    AVCodecContext* videoCodec = 0;
    AVCodecContext* audioCodec = 0;

    AVFrame* outputPicture = 0;
    AVFrame* colorSpaceConversionPicture = 0;
    struct SwsContext *colorSpaceConversionContext = 0;

    uint16_t* samples = 0;
    samplecount audioEncodeRequiredInputSize = 0;

    bool ok = true;

    if (ok && storeVideo)
    {
        videoStream = mOutputFormat->getVideoCodec()->addStream(context);
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

    }
    if (ok && storeAudio)
    {
        audioStream = mOutputFormat->getAudioCodec()->addStream(context);
        audioCodec = audioStream->codec;
    }

    av_dump_format(context, 0, filename.c_str(), 1);

    // now that all the parameters are set, we can open the audio and video codecs and allocate the necessary encode buffers
    if (ok && storeVideo)
    {
        ok = mOutputFormat->getVideoCodec()->open(videoCodec);

        outputPicture = alloc_picture(videoCodec->pix_fmt, videoCodec->width, videoCodec->height);
        ASSERT(outputPicture);

        // if the output format is not RGB24P, then a temporary picture is needed too. It is then converted to the required output format
        if (videoCodec->pix_fmt != PIX_FMT_RGB24)
        {
            colorSpaceConversionPicture = alloc_picture(PIX_FMT_RGB24, videoCodec->width, videoCodec->height);
            ASSERT(colorSpaceConversionPicture);

            static int sws_flags = SWS_BICUBIC;
            colorSpaceConversionContext = sws_getContext(videoCodec->width, videoCodec->height, PIX_FMT_RGB24, videoCodec->width, videoCodec->height, videoCodec->pix_fmt, sws_flags, 0, 0, 0);
            ASSERT_NONZERO(colorSpaceConversionContext);
        }
        videoOpened = true;
    }

    if (ok && storeAudio)
    {
        ok = mOutputFormat->getAudioCodec()->open(audioCodec);

        /* ugly hack for PCM codecs (will be removed ASAP with new PCM support to compute the input frame size in samples */
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
        audioOpened = true;
    }

    // todo reduce the number of threads.

    if (ok)
    {
        // Write the actual data into the file

        // Open the output file
        ASSERT(!(context->flags & AVFMT_NOFILE))(context);
        int result = avio_open(&context->pb, filename.c_str(), AVIO_FLAG_WRITE);
        ASSERT_MORE_THAN_EQUALS_ZERO(result);

        avformat_write_header(context,0);

        pts numberOfReadInputAudioFrames = 0;
        pts numberOfWrittenOutputAudioFrames = 0;
        pts numberOfReadInputVideoFrames = 0;
        pts numberOfWrittenOutputVideoFrames = 0;

        long lengthInMilliseconds = Convert::ptsToTime(length);
        pts lengthInVideoFrames = length;
        long maxNumberOfFrames = Convert::timeToPts(Config::ReadLong(Config::sPathDebugMaxRenderLength) *  Constants::sSecond);
        if ((maxNumberOfFrames > 0) && (lengthInVideoFrames > maxNumberOfFrames))
        {
            lengthInVideoFrames = maxNumberOfFrames;
        }
        long lengthInSeconds = Convert::ptsToTime(lengthInVideoFrames) / Constants::sSecond;

        AudioChunkPtr currentAudioChunk = sequence->getNextAudio(AudioCompositionParameters().setSampleRate(audioCodec->sample_rate).setNrChannels(audioCodec->channels));

        double audioTime = storeAudio ? 0.0 : std::numeric_limits<double>::max();
        double videoTime = storeVideo ? 0.0 : std::numeric_limits<double>::max();

        while (true)  // write interleaved audio and video frames
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
                            currentAudioChunk = sequence->getNextAudio(AudioCompositionParameters().setSampleRate(audioCodec->sample_rate).setNrChannels(audioCodec->channels));
                        }
                    }
                    else // Generate silence to fill last input packet
                    {
                        while (remainingSamples-- > 0) { *q++ = 0; }
                    }
                }

                AVFrame frame;
                frame.data[0] = (uint8_t*)samples;
                frame.linesize[0] = audioEncodeRequiredInputSize * AudioChunk::sBytesPerSample;
                frame.nb_samples = audioCodec->frame_size;
                frame.pts = numberOfReadInputAudioFrames;
                AVPacket* audioPacket = new AVPacket();
                audioPacket->data = 0;
                audioPacket->size = 0;
                int gotPacket = 0;

                // todo CODEC_CAP_DELAY see declaration  of avcodec_encode_audio2
                int result = avcodec_encode_audio2(audioCodec, audioPacket, &frame, &gotPacket); // if gotPacket == 0, then packet is destructed
                ASSERT_ZERO(result)(avcodecErrorString(result));
                numberOfReadInputAudioFrames++;

                if (gotPacket)
                {
                    audioPacket->flags |= AV_PKT_FLAG_KEY;
                    audioPacket->stream_index = audioStream->index;
                    int result = av_interleaved_write_frame(context, audioPacket); // av_interleaved_write_frame: transfers ownership of packet
                    ASSERT_ZERO(result)(avcodecErrorString(result));
                    numberOfWrittenOutputAudioFrames++;
                }
                // else Packet possibly buffered inside codec
            }
            else
            {
                // Write video frame
                if (numberOfReadInputVideoFrames >= lengthInVideoFrames )
                {
                    // no more frames to compress. The codec has a latency of a few frames if using B frames, so we get the last frames by passing the same picture again
                }
                else
                {
                    VideoFramePtr frame = sequence->getNextVideo(VideoCompositionParameters().setBoundingBox(wxSize(videoCodec->width,videoCodec->height)).setDrawBoundingBox(false));
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
                    gui::StatusBar::get().setProcessingText(ps + " " + s);
                    gui::StatusBar::get().showProgress(progress);
                    wxImagePtr image = frame->getImage();

                    if (colorSpaceConversionContext == 0)
                    {
                        memcpy(outputPicture->data[0],  image->GetData(), image->GetWidth() * image->GetHeight() * 3);
                    }
                    else
                    {
                        memcpy(colorSpaceConversionPicture->data[0],  image->GetData(), image->GetWidth() * image->GetHeight() * 3);
                        sws_scale(colorSpaceConversionContext, colorSpaceConversionPicture->data, colorSpaceConversionPicture->linesize, 0, videoCodec->height, outputPicture->data, outputPicture->linesize);
                    }
                }

                if (context->oformat->flags & AVFMT_RAWPICTURE)
                {
                    // raw video case. The API will change slightly in the near future for that
                    AVPacket* videoPacket = new AVPacket();
                    av_init_packet(videoPacket);
                    videoPacket->flags |= AV_PKT_FLAG_KEY;
                    videoPacket->stream_index= videoStream->index;
                    videoPacket->data = (uint8_t *)outputPicture;
                    videoPacket->size = sizeof(AVPicture);
                    videoPacket->pts = numberOfWrittenOutputVideoFrames;
                    int result = av_interleaved_write_frame(context, videoPacket); // av_interleaved_write_frame: transfers ownership of packet
                    ASSERT_ZERO(result)(avcodecErrorString(result));
                    numberOfWrittenOutputVideoFrames++;
                }
                else
                {
                    // encode the image
                    int gotPacket = 0;
                    AVPacket* videoPacket = new AVPacket();
                    videoPacket->data = 0;
                    videoPacket->size = 0;
                    outputPicture->pts = numberOfReadInputVideoFrames;
                    // todo handle CODEC_CAP_DELAY. NOTE: in case of failure, packet is freed here.
                    result = avcodec_encode_video2(videoCodec, videoPacket, outputPicture, &gotPacket);  // if gotPacket == 0, then packet is destructed
                    ASSERT_ZERO(result)(avcodecErrorString(result));
                    if (gotPacket)
                    {
                        int result = av_interleaved_write_frame(context, videoPacket); // av_interleaved_write_frame: transfers ownership of packet
                        ASSERT_ZERO(result)(avcodecErrorString(result));
                        numberOfWrittenOutputVideoFrames++;
                    }
                    // else Packet possibly buffered inside codec
                }
            }
        }

        int ret = av_write_trailer(context);
        ASSERT_ZERO(ret)(avcodecErrorString(ret));
    }

    if (videoOpened)
    {
        avcodec_close(videoCodec);
        av_free(outputPicture->data[0]);
        av_free(outputPicture);
        if (colorSpaceConversionPicture)
        {
            av_free(colorSpaceConversionPicture->data[0]);
            av_free(colorSpaceConversionPicture);
        }
    }

    if (audioOpened)
    {
        avcodec_close(audioCodec);
        av_free(samples);
    }

    for (unsigned int i = 0; i < context->nb_streams; i++)
    {
        av_freep(&context->streams[i]->codec);
        av_freep(&context->streams[i]);
    }

    ASSERT(!(context->flags & AVFMT_NOFILE))(context);
    avio_close(context->pb);
    av_free(context);
    gui::StatusBar::get().setProcessingText();
    gui::StatusBar::get().hideProgressBar();
}

static AVFrame *alloc_picture(enum PixelFormat pix_fmt, int width, int height)
{
    AVFrame* picture = avcodec_alloc_frame();
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

std::ostream& operator<<( std::ostream& os, const Render& obj )
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
    ar & mFileName;
    ar & mOutputFormat;
    ar & mSeparateAtCuts;
}

template void Render::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Render::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

}} //namespace