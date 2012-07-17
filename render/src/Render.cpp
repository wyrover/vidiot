#include "Render.h"

#include "stdint.h"
extern "C" {
#pragma warning(disable:4244)
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#pragma warning(default:4244)
}

#include <boost/serialization/shared_ptr.hpp>
#include <stdio.h>
#include "AudioChunk.h"
#include "AudioCodec.h"
#include "AudioCodecs.h"
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

// only 15 seconds
#define STREAM_DURATION   15.0
#define STREAM_FRAME_RATE 25 /* 25 images/s */
#define STREAM_NB_FRAMES  ((int)(STREAM_DURATION * STREAM_FRAME_RATE))
#define STREAM_PIX_FMT PIX_FMT_YUV420P /* default pix_fmt */
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
    ,   mVideoCodec(VideoCodecs::find(mOutputFormat->getDefaultVideoCodec()))
    ,   mAudioCodec(AudioCodecs::find(mOutputFormat->getDefaultAudioCodec()))
{
    VAR_DEBUG(this);
}

Render::Render(const Render& other)
    :   wxEvtHandler()
    ,   mFileName(other.mFileName)
    ,   mOutputFormat(make_cloned<OutputFormat>(other.mOutputFormat))
    ,   mVideoCodec(make_cloned<VideoCodec>(other.mVideoCodec))
    ,   mAudioCodec(make_cloned<AudioCodec>(other.mAudioCodec))
{
    VAR_DEBUG(this);
}

Render* Render::clone() const
{
    return new Render(static_cast<const Render&>(*this));
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
    // todo implement this in the dialog class
    // The file name is stripped from the comparison
    return
        (mFileName == mFileName) &&
        (*mOutputFormat == *other.mOutputFormat) &&
        (*mVideoCodec == *other.mVideoCodec) &&
        (*mAudioCodec == *other.mAudioCodec);
}

bool Render::operator!= (const Render& other) const
{
    return (!operator==(other));
}

//////////////////////////////////////////////////////////////////////////
// RENDER
//////////////////////////////////////////////////////////////////////////

void Render::generate(model::SequencePtr sequence)
{
    sequence->moveTo(0);
    int length = sequence->getLength();
    gui::StatusBar::get().showProgressBar(length);
    wxString ps; ps << _("Rendering sequence '") << sequence->getName() << "'";
    gui::StatusBar::get().setProcessingText(ps);

    AVOutputFormat* format = av_guess_format(mOutputFormat->getName().c_str(), 0, 0);
    ASSERT(format);
    ASSERT(mAudioCodec->getId() != CODEC_ID_NONE || mVideoCodec->getId() != CODEC_ID_NONE);
    format->audio_codec = mAudioCodec->getId();
    format->video_codec = mVideoCodec->getId();

    AVFormatContext* context = avformat_alloc_context();
    context->oformat = format;
    ASSERT(mFileName.IsOk());
    wxString filename = mFileName.GetFullPath();
    ASSERT_LESS_THAN_EQUALS(sizeof(filename.c_str()),sizeof(context->filename));
    _snprintf(context->filename, sizeof(context->filename), "%s", filename.c_str());

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

    if (format->video_codec != CODEC_ID_NONE)
    {
        video_stream = av_new_stream(context, 0);
        ASSERT(video_stream); // todo proper error handling instead?

        AVCodecContext* video_codec = video_stream->codec;
        video_codec->codec_id = format->video_codec;
        video_codec->codec_type = AVMEDIA_TYPE_VIDEO;

        mVideoCodec->setParameters(video_codec);
        //video_codec->width = 352; // resolution must be a multiple of two
        //video_codec->height = 288;
        //video_codec->thread_count = 3; // todo
        video_codec->width = Properties::get()->getVideoSize().GetWidth(); // resolution must be a multiple of two
        video_codec->height = Properties::get()->getVideoSize().GetHeight();
        // todo check the bitrate, see if it's being set
        // todo assert a certain minimum size
        // todo asserts for even numbers

        // Fundamental unit of time (in seconds) in terms of which frame timestamps are represented.
        // For fixed-fps content, timebase should be 1/framerate and timestamp increments should be identically 1.
        video_codec->time_base.den = Properties::get()->getFrameRate().denominator();
        video_codec->time_base.num = Properties::get()->getFrameRate().numerator();
        video_codec->gop_size = 12; /* emit one intra frame every twelve frames at most */
        video_codec->pix_fmt = STREAM_PIX_FMT;
        // Some formats want stream headers to be separate
        if (context->oformat->flags & AVFMT_GLOBALHEADER)
        {
            video_codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
        }
    }
    if (format->audio_codec != CODEC_ID_NONE)
    {
        AVCodecContext* audio_codec;
        audio_stream = av_new_stream(context, 1);
        ASSERT(audio_stream);
        audio_codec = audio_stream->codec;
        // todo set the audiocodec params
        audio_codec->codec_id = format->audio_codec;
        audio_codec->codec_type = AVMEDIA_TYPE_AUDIO;
        audio_codec->sample_fmt = AV_SAMPLE_FMT_S16;
        audio_codec->bit_rate = 64000;
        mAudioCodec->setParameters(audio_codec);
        audio_codec->sample_rate = Properties::get()->getAudioFrameRate();
        audio_codec->channels = Properties::get()->getAudioNumberOfChannels();
        mAudioCodec->setParameters(audio_codec);

        // todo this must be handled in 'audiocodec::setParameters...'?
        // some formats want stream headers to be separate
        if (context->oformat->flags & AVFMT_GLOBALHEADER)
        {
            audio_codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
        }
    }

    av_dump_format(context, 0, filename.c_str(), 1);

    // now that all the parameters are set, we can open the audio and video codecs and allocate the necessary encode buffers
    if (video_stream)
    {
        AVCodecContext* video_codec_context = video_stream->codec;
        AVCodec* codec = avcodec_find_encoder(video_codec_context->codec_id);
        ASSERT(codec);

        int result = avcodec_open(video_codec_context, codec);

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

        picture = alloc_picture(video_codec_context->pix_fmt, video_codec_context->width, video_codec_context->height);
        ASSERT(picture);

        //// if the output format is not RGB24P, then a temporary picture is needed too. It is then converted to the required output format
        if (video_codec_context->pix_fmt != PIX_FMT_RGB24)
        {
            tmp_picture = alloc_picture(PIX_FMT_RGB24, video_codec_context->width, video_codec_context->height);
            ASSERT(tmp_picture);

        }
    }

    if (audio_stream)
    {
        AVCodecContext *audio_codec_context = audio_stream->codec;
        AVCodec* audio_codec = avcodec_find_encoder(audio_codec_context->codec_id);
        ASSERT(audio_codec);

        int result = avcodec_open(audio_codec_context, audio_codec);
        ASSERT_MORE_THAN_EQUALS_ZERO(result);

        audio_outbuf_size = 10000;
        audio_outbuf = (uint8_t*)av_malloc(audio_outbuf_size);

        /* ugly hack for PCM codecs (will be removed ASAP with new PCM support to compute the input frame size in samples */
        if (audio_codec_context->frame_size <= 1)
        {
            audio_input_frame_size = audio_outbuf_size / audio_codec_context->channels;
            switch (audio_codec_context->codec_id)
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
            audio_input_frame_size = audio_codec_context->frame_size;
        }
        samples = (int16_t*)av_malloc(Convert::audioFramesToBytes(audio_input_frame_size, audio_codec_context->channels));
    }

    // open the output file, if needed
    if (!(format->flags & AVFMT_NOFILE))
    {
        int result = avio_open(&context->pb, filename.c_str(), AVIO_FLAG_WRITE);
        ASSERT_MORE_THAN_EQUALS_ZERO(result);
    }

    avformat_write_header(context,0);

    double audio_pts = 0.0;
    double video_pts = 0.0;

    bool done = false;

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

        if ((!audio_stream || audio_pts >= STREAM_DURATION) && (!video_stream || video_pts >= STREAM_DURATION))
        {
            break;
        }

        // write interleaved audio and video frames
        if (!video_stream || (video_stream && audio_stream && audio_pts < video_pts))
        {
            // Write audio frame
            AVCodecContext *audio_codec_context = audio_stream->codec;
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

            pkt.size = avcodec_encode_audio(audio_codec_context, audio_outbuf, audio_outbuf_size, samples);
            if (audio_codec_context->coded_frame && audio_codec_context->coded_frame->pts != AV_NOPTS_VALUE)
            {
                pkt.pts = av_rescale_q(audio_codec_context->coded_frame->pts, audio_codec_context->time_base, audio_stream->time_base);
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
            AVCodecContext* video_codec_context = video_stream->codec;
            struct SwsContext *img_convert_ctx = 0;

            if (frame_count >= STREAM_NB_FRAMES)
            {
                // no more frame to compress. The codec has a latency of a few frames if using B frames, so we get the last frames by passing the same picture again
            }
            else
            {
                VideoFramePtr frame = sequence->getNextVideo(VideoCompositionParameters().setBoundingBox(wxSize(video_codec_context->width,video_codec_context->height)).setDrawBoundingBox(false));
                wxString s; s << _("(frame ") << frame->getPts() << _(" out of ") << length << ")";
                gui::StatusBar::get().setProcessingText(ps + " " + s);
                gui::StatusBar::get().showProgress(frame->getPts());

                if (video_codec_context->pix_fmt != PIX_FMT_RGB24)
                {
                    // Convert to desired pixel format
                    if (img_convert_ctx == 0)
                    {
                        img_convert_ctx = sws_getContext(video_codec_context->width, video_codec_context->height,
                            PIX_FMT_RGB24,
                            video_codec_context->width, video_codec_context->height,
                            video_codec_context->pix_fmt,
                            sws_flags, 0, 0, 0);
                        ASSERT_NONZERO(img_convert_ctx);

                        wxImagePtr image = frame->getImage();
                        memcpy(tmp_picture->data[0],  image->GetData(), image->GetWidth() * image->GetHeight() * 3);

                        sws_scale(img_convert_ctx, tmp_picture->data, tmp_picture->linesize, 0, video_codec_context->height, picture->data, picture->linesize);
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
                out_size = avcodec_encode_video(video_codec_context, video_outbuf, video_outbuf_size, picture);
                // if zero size, it means the image was buffered
                if (out_size > 0)
                {
                    AVPacket pkt;
                    av_init_packet(&pkt);

                    if (video_codec_context->coded_frame->pts != AV_NOPTS_VALUE)
                    {
                        pkt.pts= av_rescale_q(video_codec_context->coded_frame->pts, video_codec_context->time_base, video_stream->time_base);
                    }
                    if(video_codec_context->coded_frame->key_frame)
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

    if (!(format->flags & AVFMT_NOFILE))
    {
        avio_close(context->pb);
    }
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

VideoCodecPtr Render::getVideoCodec() const
{
    return mVideoCodec;
}

void Render::setVideoCodec(VideoCodecPtr codec)
{
    mVideoCodec = codec;
}

AudioCodecPtr Render::getAudioCodec() const
{
    return mAudioCodec;
}

void Render::setAudioCodec(AudioCodecPtr codec)
{
    mAudioCodec = codec;
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
    filename.ClearExt();
    filename.SetName("");
    cloned->setFileName(filename);
    return cloned;
}

//////////////////////////////////////////////////////////////////////////
// RENDERING
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
    error << _("The following sequence(s) have no configured filename and have not been scheduled:");
    std::list<wxString> errors;
    BOOST_FOREACH( SequencePtr sequence, seqs )
    {
        if (!sequence->getRender()->checkFileName())
        {
            error << '\n' << "- " << sequence->getName() << '\n';
            anError = true;
        }
        else
        {
            schedule(sequence);
        }
    }
    wxMessageBox(error, _("Not all sequences have been scheduled"),wxOK);
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const Render& obj )
{
    os  << &obj           << '|'
        << obj.mFileName << '|'
        << obj.mOutputFormat;
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
    ar & mVideoCodec;
    ar & mAudioCodec;
}
template void Render::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Render::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

}} //namespace