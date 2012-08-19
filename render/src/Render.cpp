#include "Render.h"

#include "AudioChunk.h"
#include "AudioCodec.h"
#include "AudioCodecs.h"
#include "Config.h"
#include "Constants.h"
#include "Convert.h"
#include "Folder.h"
#include "StatusBar.h"
#include "OutputFormat.h"
#include "OutputFormats.h"
#include "Project.h"
#include "Properties.h"
#include "Sequence.h"
#include "UtilLog.h"
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

static int sws_flags = SWS_BICUBIC;

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
{
    VAR_DEBUG(this);
}

Render::Render(const Render& other)
    :   wxEvtHandler()
    ,   mFileName(other.mFileName)
    ,   mOutputFormat(make_cloned<OutputFormat>(other.mOutputFormat))
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
    model::SequencePtr clone = make_cloned<model::Sequence>(sequence);
    gui::Worker::get().schedule(boost::make_shared<Work>(boost::bind(&Render::generate,clone->getRender(),clone)));
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

void Render::generate(model::SequencePtr sequence)
{
    sequence->moveTo(0);
    int length = sequence->getLength();
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
    AVStream* video_stream = 0;
    AVStream* audio_stream = 0;

    uint8_t *video_outbuf = 0;
    int frame_count = 0;
    int video_outbuf_size = 0;
    AVFrame* picture = 0;
    AVFrame* tmp_picture = 0;

    int16_t* samples = 0;
    uint8_t* audio_outbuf = 0;
    int audio_outbuf_size = 0;
    int audio_input_frame_size = 0;
    int bytesPerAudioFrame = 0;

    if (storeVideo)
    {
        video_stream = mOutputFormat->getVideoCodec()->addStream(context);
    }
    if (storeAudio)
    {
        audio_stream = mOutputFormat->getAudioCodec()->addStream(context);
    }

    av_dump_format(context, 0, filename.c_str(), 1);

    // now that all the parameters are set, we can open the audio and video codecs and allocate the necessary encode buffers
    if (storeVideo)
    {
        mOutputFormat->getVideoCodec()->open(video_stream->codec);

        video_outbuf = 0;
        if (!(context->oformat->flags & AVFMT_RAWPICTURE))
        {
            /* allocate output buffer */
            /* XXX: API change will be done */
            /* buffers passed into lav* can be allocated any way you prefer,
            as long as they're aligned enough for the architecture, and
            they're freed appropriately (such as using av_free for buffers
            allocated with av_malloc) */
            video_outbuf_size = 200000;
            video_outbuf = (uint8_t*)av_malloc(video_outbuf_size);
        }

        picture = alloc_picture(video_stream->codec->pix_fmt, video_stream->codec->width, video_stream->codec->height);
        ASSERT(picture);

        //// if the output format is not RGB24P, then a temporary picture is needed too. It is then converted to the required output format
        if (video_stream->codec->pix_fmt != PIX_FMT_RGB24)
        {
            tmp_picture = alloc_picture(PIX_FMT_RGB24, video_stream->codec->width, video_stream->codec->height);
            ASSERT(tmp_picture);

        }
    }

    if (storeAudio)
    {
        mOutputFormat->getAudioCodec()->open(audio_stream->codec);

        audio_outbuf_size = 10000;
        audio_outbuf = (uint8_t*)av_malloc(audio_outbuf_size);

        /* ugly hack for PCM codecs (will be removed ASAP with new PCM support to compute the input frame size in samples */
        if (audio_stream->codec->frame_size <= 1)
        {
            audio_input_frame_size = audio_outbuf_size / audio_stream->codec->channels;
            switch (audio_stream->codec->codec_id)
            {
            case CODEC_ID_PCM_S16LE:
            case CODEC_ID_PCM_S16BE:
            case CODEC_ID_PCM_U16LE:
            case CODEC_ID_PCM_U16BE:
                audio_input_frame_size >>= 1;
                break;
            default:
                break;
            }
        }
        else
        {
            audio_input_frame_size = audio_stream->codec->frame_size;
        }
        samples = (int16_t*)av_malloc(Convert::audioFramesToBytes(audio_input_frame_size, audio_stream->codec->channels));
    }

    // Open the output file
    ASSERT(!(context->flags & AVFMT_NOFILE))(context);
    int result = avio_open(&context->pb, filename.c_str(), AVIO_FLAG_WRITE);
    ASSERT_MORE_THAN_EQUALS_ZERO(result);

    avformat_write_header(context,0);

    double audio_pts = 0.0;
    double video_pts = 0.0;

    bool done = false;

    long lengthInMilliseconds = Convert::ptsToTime(length);
    pts lengthInVideoFrames = length;
    long maxNumberOfFrames = Convert::timeToPts(Config::ReadLong(Config::sPathDebugMaxRenderLength) *  Constants::sSecond);
    if ((maxNumberOfFrames > 0) && (lengthInVideoFrames > maxNumberOfFrames))
    {
        lengthInVideoFrames = maxNumberOfFrames;
    }
    long lengthInSeconds = Convert::ptsToTime(lengthInVideoFrames) / Constants::sSecond;

    AudioChunkPtr currentAudioChunk = sequence->getNextAudio(audio_stream->codec->sample_rate,audio_stream->codec->channels);
    while (!done)
    {
        if (audio_stream)
        {
            audio_pts = (double)audio_stream->pts.val * audio_stream->time_base.num / audio_stream->time_base.den;
        }
        if (video_stream)
        {
            video_pts = (double)video_stream->pts.val * video_stream->time_base.num / video_stream->time_base.den;
        }

        if ((!audio_stream || audio_pts >= lengthInSeconds) && (!video_stream || video_pts >= lengthInSeconds))
        {
            break;
        }

        // write interleaved audio and video frames
        if (!video_stream || (video_stream && audio_stream && audio_pts < video_pts))
        {
            // Write audio frame
            AVPacket pkt;
            av_init_packet(&pkt);

            int16_t* q = samples;
            samplecount remainingSamples = audio_input_frame_size * 2; // 2 -> stereo

            while (remainingSamples > 0)
            {
                if (currentAudioChunk)
                {
                    // Previous chunk not used completely
                    //samplecount remainingSamples = audio_input_frame_size - written;
                    samplecount nSamples = min(remainingSamples, currentAudioChunk->getUnreadSampleCount());

                    memcpy(q,currentAudioChunk->getUnreadSamples(), nSamples * 2); // todo add this to the audiochunk class and then reuse in videodisplay also? // the 2  here is for bytespersample
                    currentAudioChunk->read(nSamples);                                                              // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^alsothis^^^^^^^^^^^^^^^^
                    q += nSamples;
                    remainingSamples -= nSamples;

                    if (currentAudioChunk->getUnreadSampleCount() == 0)
                    {
                        currentAudioChunk = sequence->getNextAudio(audio_stream->codec->sample_rate,audio_stream->codec->channels);
                    }
                }
                else
                {
                    // Generate silence
                    while (remainingSamples > 0)
                    {
                        *q++ = 0;
                        remainingSamples--;
                    }
                }
            }

            pkt.size = avcodec_encode_audio(audio_stream->codec, audio_outbuf, audio_outbuf_size, samples);
            if (audio_stream->codec->coded_frame && audio_stream->codec->coded_frame->pts != AV_NOPTS_VALUE)
            {
                pkt.pts = av_rescale_q(audio_stream->codec->coded_frame->pts, audio_stream->codec->time_base, audio_stream->time_base);
            }
            pkt.flags |= AV_PKT_FLAG_KEY;
            pkt.stream_index = audio_stream->index;
            pkt.data = audio_outbuf;

            // write the compressed frame in the media file/
            int result = av_interleaved_write_frame(context, &pkt);
            ASSERT_ZERO(result);
        }
        else
        {
            // Write video frame

            int out_size, ret;
            struct SwsContext *img_convert_ctx = 0;

            if (frame_count >= lengthInVideoFrames)
            {
                // no more frame to compress. The codec has a latency of a few frames if using B frames, so we get the last frames by passing the same picture again
            }
            else
            {
                VideoFramePtr frame = sequence->getNextVideo(VideoCompositionParameters().setBoundingBox(wxSize(video_stream->codec->width,video_stream->codec->height)).setDrawBoundingBox(false));
                wxString s; s << _("(frame ") << frame->getPts() << _(" out of ") << length << ")";
                gui::StatusBar::get().setProcessingText(ps + " " + s);
                gui::StatusBar::get().showProgress(frame->getPts());

                if (video_stream->codec->pix_fmt != PIX_FMT_RGB24)
                {
                    // Convert to desired pixel format
                    if (img_convert_ctx == 0)
                    {
                        img_convert_ctx = sws_getContext(video_stream->codec->width, video_stream->codec->height,
                            PIX_FMT_RGB24,
                            video_stream->codec->width, video_stream->codec->height,
                            video_stream->codec->pix_fmt,
                            sws_flags, 0, 0, 0);
                        ASSERT_NONZERO(img_convert_ctx);

                        wxImagePtr image = frame->getImage();
                        memcpy(tmp_picture->data[0],  image->GetData(), image->GetWidth() * image->GetHeight() * 3);

                        sws_scale(img_convert_ctx, tmp_picture->data, tmp_picture->linesize, 0, video_stream->codec->height, picture->data, picture->linesize);
                    }
                }
            }

            if (context->oformat->flags & AVFMT_RAWPICTURE)
            {
                // raw video case. The API will change slightly in the near futur for that
                AVPacket pkt;
                av_init_packet(&pkt);
                pkt.flags |= AV_PKT_FLAG_KEY;
                pkt.stream_index= video_stream->index;
                pkt.data= (uint8_t *)picture;
                pkt.size= sizeof(AVPicture);
                ret = av_interleaved_write_frame(context, &pkt);
            }
            else
            {
                // encode the image
                out_size = avcodec_encode_video(video_stream->codec, video_outbuf, video_outbuf_size, picture);
                // if zero size, it means the image was buffered
                if (out_size > 0)
                {
                    AVPacket pkt;
                    av_init_packet(&pkt);

                    if (video_stream->codec->coded_frame->pts != AV_NOPTS_VALUE)
                    {
                        pkt.pts= av_rescale_q(video_stream->codec->coded_frame->pts, video_stream->codec->time_base, video_stream->time_base);
                    }
                    if(video_stream->codec->coded_frame->key_frame)
                    {
                        pkt.flags |= AV_PKT_FLAG_KEY;
                    }
                    pkt.stream_index= video_stream->index;
                    pkt.data= video_outbuf;
                    pkt.size= out_size;

                    // write the compressed frame in the media file
                    ret = av_interleaved_write_frame(context, &pkt);
                }
                else
                {
                    ret = 0;
                }
            }

            ASSERT_ZERO(ret);
            frame_count++;
        }
    }

    av_write_trailer(context);

    if (video_stream)
    {
        avcodec_close(video_stream->codec);
        av_free(picture->data[0]);
        av_free(picture);
        if (tmp_picture)
        {
            av_free(tmp_picture->data[0]);
            av_free(tmp_picture);
        }
        av_free(video_outbuf);
    }

    if (audio_stream)
    {
        avcodec_close(audio_stream->codec);
        av_free(samples);
        av_free(audio_outbuf);
    }

    // free the streams
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

//You'll need to look at the libavcodec documentation - specifically, at avcodec_encode_video(). I found that the best available documentation is in the ffmpeg header files and the API sample source code that's provided with the ffmpeg source. Specifically, look at libavcodec/api-example.c or even ffmpeg.c.
//
// todo set i frames for each first (and last?) frame of a clip boundary..
//To force an I frame, you'll need to set the pict_type member of the picture you're encoding to 1: 1 is an I frame, 2 is a P frame, and I don't remember what's the code for a B frame off the top of my head... Also, the key_frame member needs to be set to 1.
//You'll need to be careful how you allocate the frame objects that the API calls require. api-example.c is your best bet as far as that goes, in my opinion. Look for the function video_encode_example() - it's concise and illustrates all the important things you need to worry about - pay special attention to the second call to avcodec_encode_video() that passes a NULL picture argument - it's required to get the last frames of video since MPEG video is encoded out of sequence and you may end up with a delay of a few frames.
//
//Btw, the values for the pict_type member of the picture are AV_PICTURE_TYPE_I, AV_PICTURE_TYPE_P, AV_PICTURE_TYPE_B, and so o

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
    os  << &obj           << '|'
        << obj.mFileName << '|'
        << *obj.mOutputFormat;
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
}
template void Render::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Render::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

}} //namespace