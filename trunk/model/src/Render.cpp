#include "Render.h"

// Include at top, to exclude the intmax macros and use the boost versions
//#include "stdint.h"
#define BOOST_HAS_STDINT_H

#undef INTMAX_C
#undef UINTMAX_C
#define CONFIG_SWSCALE_ALPHA
extern "C" {
#include <swscale.h>
#include <avformat.h>
}
#include <stdio.h>
#include "AudioChunk.h"
#include "Properties.h"
#include "Sequence.h"
#include "UtilLog.h"
#include "VideoFrame.h"
#include "VideoCompositionParameters.h"

namespace model {

static AVFrame *alloc_picture(enum PixelFormat pix_fmt, int width, int height);
static void get_audio_frame(int16_t *samples, int frame_size, int nb_channels, float& t, float& tincr, float& tincr2);
static void fill_yuv_image(AVFrame *pict, int frame_index, int width, int height);

// only 5 seconds
#define STREAM_DURATION   5.0
#define STREAM_FRAME_RATE 25 /* 25 images/s */
#define STREAM_NB_FRAMES  ((int)(STREAM_DURATION * STREAM_FRAME_RATE))
#define STREAM_PIX_FMT PIX_FMT_YUV420P /* default pix_fmt */
static int sws_flags = SWS_BICUBIC;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Render::Render(SequencePtr sequence)
    :   mSequence(sequence)
{
    VAR_DEBUG(this);
}

Render::~Render()
{
    VAR_DEBUG(this);
}

void Render::generate()
{
    std::string filename("D:\\out.avi");
    //AVOutputFormat* format = av_guess_format("mpeg", NULL, NULL);
    AVOutputFormat* format = av_guess_format("vob", NULL, NULL);
    ASSERT(format);
    //int result = avformat_alloc_output_context2(AVFormatContext **ctx, AVOutputFormat *oformat, const char *format_name, const char *filename);

    AVFormatContext* context = avformat_alloc_context();
    context->oformat = format;
    _snprintf(context->filename, sizeof(context->filename), "%s", filename.c_str());

    AVStream* video_stream = 0;
    AVStream* audio_stream = 0;

    uint8_t *video_outbuf = 0;
    int frame_count = 0;
    int video_outbuf_size = 0;
    AVFrame* picture = 0;
    AVFrame* tmp_picture = 0;

    float t = 0;
    float tincr = 0;
    float tincr2 = 0;
    int16_t* samples = 0;
    uint8_t* audio_outbuf = 0;
    int audio_outbuf_size = 0;
    int audio_input_frame_size = 0;
    int bytesPerAudioFrame = 0;

    mSequence->moveTo(0);

    if (format->video_codec != CODEC_ID_NONE)
    {
        video_stream = av_new_stream(context, 0);
        ASSERT(video_stream); // todo proper error handling instead?

        AVCodecContext* video_codec = video_stream->codec;
        video_codec->codec_id = format->video_codec;
        video_codec->codec_type = AVMEDIA_TYPE_VIDEO;
        video_codec->bit_rate = 4000000; // todo configurable
        //video_codec->width = 352; // resolution must be a multiple of two
        //video_codec->height = 288;
        video_codec->width = Properties::get()->getVideoSize().GetWidth(); // resolution must be a multiple of two
        video_codec->height = Properties::get()->getVideoSize().GetHeight();
        // todo assert a certain minimum size
        // todo asserts for even numbers

        // Fundamental unit of time (in seconds) in terms of which frame timestamps are represented.
        // For fixed-fps content, timebase should be 1/framerate and timestamp increments should be identically 1.
        video_codec->time_base.den = STREAM_FRAME_RATE;
        video_codec->time_base.num = 1;
        video_codec->gop_size = 12; /* emit one intra frame every twelve frames at most */
        video_codec->pix_fmt = STREAM_PIX_FMT;
        if (video_codec->codec_id == CODEC_ID_MPEG2VIDEO)
        {
            video_codec->max_b_frames = 2; // just for testing, we also add B frames
        }
        if (video_codec->codec_id == CODEC_ID_MPEG1VIDEO)
        {
            // Needed to avoid using macroblocks in which some coeffs overflow.
            // This does not happen with normal video, it just happens here as
            // the motion of the chroma plane does not match the luma plane.
            video_codec->mb_decision=2;
        }
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
        audio_codec->codec_id = format->audio_codec;
        audio_codec->codec_type = AVMEDIA_TYPE_AUDIO;
        audio_codec->sample_fmt = AV_SAMPLE_FMT_S16;
        audio_codec->bit_rate = 64000;
        audio_codec->sample_rate = 44100;
        audio_codec->channels = 2;
        // some formats want stream headers to be separate
        if (context->oformat->flags & AVFMT_GLOBALHEADER)
        {
            audio_codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
        }
    }

    // set the output parameters (must be done even if no parameters)
    int result = av_set_parameters(context, 0);
    ASSERT_MORE_THAN_EQUALS_ZERO(result); // else Invalid output format parameters
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

        //// if the output format is not RGB24P, then a temporary YUV420P picture is needed too. It is then converted to the required output format
        if (video_codec_context->pix_fmt != PIX_FMT_RGB24)
        {
            tmp_picture = alloc_picture(PIX_FMT_RGB24, video_codec_context->width, video_codec_context->height);
            ASSERT(tmp_picture);

        }

        //// if the output format is not YUV420P, then a temporary YUV420P picture is needed too. It is then converted to the required output format
        //if (video_codec_context->pix_fmt != PIX_FMT_YUV420P)
        //{
        //    tmp_picture = alloc_picture(PIX_FMT_YUV420P, video_codec_context->width, video_codec_context->height);
        //    ASSERT(tmp_picture);

        //}
    }

    if (audio_stream)
    {
        AVCodecContext *audio_codec_context = audio_stream->codec;
        AVCodec* audio_codec = avcodec_find_encoder(audio_codec_context->codec_id);
        ASSERT(audio_codec);

        int result = avcodec_open(audio_codec_context, audio_codec);
        ASSERT_MORE_THAN_EQUALS_ZERO(result);

        /* init signal generator */
        t = 0;
        tincr = 2 * M_PI * 110.0 / audio_codec_context->sample_rate;
        /* increment frequency by 110 Hz per second */
        tincr2 = 2 * M_PI * 110.0 / audio_codec_context->sample_rate / audio_codec_context->sample_rate;

        bytesPerAudioFrame = model::AudioChunk::sBytesPerSample * audio_codec_context->channels;

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
        samples = (int16_t*)av_malloc(audio_input_frame_size * bytesPerAudioFrame);
    }

    // open the output file, if needed
    if (!(format->flags & AVFMT_NOFILE))
    {
        int result = avio_open(&context->pb, filename.c_str(), URL_WRONLY);
        ASSERT_MORE_THAN_EQUALS_ZERO(result);
    }

    av_write_header(context);

    double audio_pts = 0.0;
    double video_pts = 0.0;

    bool done = false;

    AudioChunkPtr currentAudioChunk = mSequence->getNextAudio(audio_stream->codec->sample_rate,audio_stream->codec->channels);
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
                        currentAudioChunk = mSequence->getNextAudio(audio_stream->codec->sample_rate,audio_stream->codec->channels);
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

            //get_audio_frame(samples, audio_input_frame_size, audio_codec_context->channels, t, tincr, tincr2);

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
                //break;// todo had to add this
            }
            else
            {
                VideoFramePtr frame = mSequence->getNextVideo(VideoCompositionParameters().setBoundingBox(wxSize(video_codec_context->width,video_codec_context->height)).setDrawBoundingBox(false));

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
            //            fill_yuv_image(tmp_picture, frame_count, video_codec_context->width, video_codec_context->height);

                        sws_scale(img_convert_ctx, tmp_picture->data, tmp_picture->linesize, 0, video_codec_context->height, picture->data, picture->linesize);
                    }
                }
                //if (video_codec_context->pix_fmt != PIX_FMT_YUV420P)
                //{
                //    // as we only generate a YUV420P picture, we must convert it to the codec pixel format if needed
                //    if (img_convert_ctx == 0)
                //    {
                //        img_convert_ctx = sws_getContext(video_codec_context->width, video_codec_context->height,
                //            PIX_FMT_YUV420P,
                //            video_codec_context->width, video_codec_context->height,
                //            video_codec_context->pix_fmt,
                //            sws_flags, 0, 0, 0);
                //        ASSERT_NONZERO(img_convert_ctx);

                //        fill_yuv_image(tmp_picture, frame_count, video_codec_context->width, video_codec_context->height);

                //        sws_scale(img_convert_ctx, tmp_picture->data, tmp_picture->linesize, 0, video_codec_context->height, picture->data, picture->linesize);
                //    }
                //    else
                //    {
                //        fill_yuv_image(picture, frame_count, video_codec_context->width, video_codec_context->height);
                //    }
                //}
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
}

//00048 static void audio_encode_example(const char *filename)
//00049 {
//00050     AVCodec *codec;
//00051     AVCodecContext *c= NULL;
//00052     int frame_size, i, j, out_size, outbuf_size;
//00053     FILE *f;
//00054     short *samples;
//00055     float t, tincr;
//00056     uint8_t *outbuf;
//00057
//00058     printf("Audio encoding\n");
//00059
//00060     /* find the MP2 encoder */
//00061     codec = avcodec_find_encoder(CODEC_ID_MP2);
//00062     if (!codec) {
//00063         fprintf(stderr, "codec not found\n");
//00064         exit(1);
//00065     }
//00066
//00067     c= avcodec_alloc_context();
//00068
//00069     /* put sample parameters */
//00070     c->bit_rate = 64000;
//00071     c->sample_rate = 44100;
//00072     c->channels = 2;
//00073
//00074     /* open it */
//00075     if (avcodec_open(c, codec) < 0) {
//00076         fprintf(stderr, "could not open codec\n");
//00077         exit(1);
//00078     }
//00079
//00080     /* the codec gives us the frame size, in samples */
//00081     frame_size = c->frame_size;
//00082     samples = malloc(frame_size * 2 * c->channels);
//00083     outbuf_size = 10000;
//00084     outbuf = malloc(outbuf_size);
//00085
//00086     f = fopen(filename, "wb");
//00087     if (!f) {
//00088         fprintf(stderr, "could not open %s\n", filename);
//00089         exit(1);
//00090     }
//00091
//00092     /* encode a single tone sound */
//00093     t = 0;
//00094     tincr = 2 * M_PI * 440.0 / c->sample_rate;
//00095     for(i=0;i<200;i++) {
//00096         for(j=0;j<frame_size;j++) {
//00097             samples[2*j] = (int)(sin(t) * 10000);
//00098             samples[2*j+1] = samples[2*j];
//00099             t += tincr;
//00100         }
//00101         /* encode the samples */
//00102         out_size = avcodec_encode_audio(c, outbuf, outbuf_size, samples);
//00103         fwrite(outbuf, 1, out_size, f);
//00104     }
//00105     fclose(f);
//00106     free(outbuf);
//00107     free(samples);
//00108
//00109     avcodec_close(c);
//00110     av_free(c);
//00111 }
//00112
//

//You'll need to look at the libavcodec documentation - specifically, at avcodec_encode_video(). I found that the best available documentation is in the ffmpeg header files and the API sample source code that's provided with the ffmpeg source. Specifically, look at libavcodec/api-example.c or even ffmpeg.c.
//
//To force an I frame, you'll need to set the pict_type member of the picture you're encoding to 1: 1 is an I frame, 2 is a P frame, and I don't remember what's the code for a B frame off the top of my head... Also, the key_frame member needs to be set to 1.
//You'll need to be careful how you allocate the frame objects that the API calls require. api-example.c is your best bet as far as that goes, in my opinion. Look for the function video_encode_example() - it's concise and illustrates all the important things you need to worry about - pay special attention to the second call to avcodec_encode_video() that passes a NULL picture argument - it's required to get the last frames of video since MPEG video is encoded out of sequence and you may end up with a delay of a few frames.
//
//Btw, the values for the pict_type member of the picture are AV_PICTURE_TYPE_I, AV_PICTURE_TYPE_P, AV_PICTURE_TYPE_B, and so o

//00200 static void video_encode_example(const char *filename)
//00201 {
//00202     AVCodec *codec;
//00203     AVCodecContext *c= NULL;
//00204     int i, out_size, size, x, y, outbuf_size;
//00205     FILE *f;
//00206     AVFrame *picture;
//00207     uint8_t *outbuf, *picture_buf;
//00208
//00209     printf("Video encoding\n");
//00210
//00211     /* find the mpeg1 video encoder */
//00212     codec = avcodec_find_encoder(CODEC_ID_MPEG1VIDEO);
//00213     if (!codec) {
//00214         fprintf(stderr, "codec not found\n");
//00215         exit(1);
//00216     }
//00217
//00218     c= avcodec_alloc_context();
//00219     picture= avcodec_alloc_frame();
//00220
//00221     /* put sample parameters */
//00222     c->bit_rate = 400000;
//00223     /* resolution must be a multiple of two */
//00224     c->width = 352;
//00225     c->height = 288;
//00226     /* frames per second */
//00227     c->time_base= (AVRational){1,25};
//00228     c->gop_size = 10; /* emit one intra frame every ten frames */
//00229     c->max_b_frames=1;
//00230     c->pix_fmt = PIX_FMT_YUV420P;
//00231
//00232     /* open it */
//00233     if (avcodec_open(c, codec) < 0) {
//00234         fprintf(stderr, "could not open codec\n");
//00235         exit(1);
//00236     }
//00237
//00238     f = fopen(filename, "wb");
//00239     if (!f) {
//00240         fprintf(stderr, "could not open %s\n", filename);
//00241         exit(1);
//00242     }
//00243
//00244     /* alloc image and output buffer */
//00245     outbuf_size = 100000;
//00246     outbuf = malloc(outbuf_size);
//00247     size = c->width * c->height;
//00248     picture_buf = malloc((size * 3) / 2); /* size for YUV 420 */
//00249
//00250     picture->data[0] = picture_buf;
//00251     picture->data[1] = picture->data[0] + size;
//00252     picture->data[2] = picture->data[1] + size / 4;
//00253     picture->linesize[0] = c->width;
//00254     picture->linesize[1] = c->width / 2;
//00255     picture->linesize[2] = c->width / 2;
//00256
//00257     /* encode 1 second of video */
//00258     for(i=0;i<25;i++) {
//00259         fflush(stdout);
//00260         /* prepare a dummy image */
//00261         /* Y */
//00262         for(y=0;y<c->height;y++) {
//00263             for(x=0;x<c->width;x++) {
//00264                 picture->data[0][y * picture->linesize[0] + x] = x + y + i * 3;
//00265             }
//00266         }
//00267
//00268         /* Cb and Cr */
//00269         for(y=0;y<c->height/2;y++) {
//00270             for(x=0;x<c->width/2;x++) {
//00271                 picture->data[1][y * picture->linesize[1] + x] = 128 + y + i * 2;
//00272                 picture->data[2][y * picture->linesize[2] + x] = 64 + x + i * 5;
//00273             }
//00274         }
//00275
//00276         /* encode the image */
//00277         out_size = avcodec_encode_video(c, outbuf, outbuf_size, picture);
//00278         printf("encoding frame %3d (size=%5d)\n", i, out_size);
//00279         fwrite(outbuf, 1, out_size, f);
//00280     }
//00281
//00282     /* get the delayed frames */
//00283     for(; out_size; i++) {
//00284         fflush(stdout);
//00285
//00286         out_size = avcodec_encode_video(c, outbuf, outbuf_size, NULL);
//00287         printf("write frame %3d (size=%5d)\n", i, out_size);
//00288         fwrite(outbuf, 1, out_size, f);
//00289     }
//00290
//00291     /* add sequence end code to have a real mpeg file */
//00292     outbuf[0] = 0x00;
//00293     outbuf[1] = 0x00;
//00294     outbuf[2] = 0x01;
//00295     outbuf[3] = 0xb7;
//00296     fwrite(outbuf, 1, 4, f);
//00297     fclose(f);
//00298     free(picture_buf);
//00299     free(outbuf);
//00300
//00301     avcodec_close(c);
//00302     av_free(c);
//00303     av_free(picture);
//00304     printf("\n");
//00305 }
//00306
//

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

static void get_audio_frame(int16_t *samples, int frame_size, int nb_channels, float& t, float &tincr, float &tincr2)
{
    int j, i, v;
    int16_t *q;
    q = samples;
    for(j=0;j<frame_size;j++)
    {
        v = (int)(sin(t) * 10000);
        for(i = 0; i < nb_channels; i++)
            *q++ = v;
        t += tincr;
        tincr += tincr2;
    }
}

static void fill_yuv_image(AVFrame *pict, int frame_index, int width, int height)
{
    int x, y, i;

    i = frame_index;

    /* Y */
    for(y=0;y<height;y++) {
        for(x=0;x<width;x++) {
            pict->data[0][y * pict->linesize[0] + x] = x + y + i * 3;
        }
    }

    /* Cb and Cr */
    for(y=0;y<height/2;y++) {
        for(x=0;x<width/2;x++) {
            pict->data[1][y * pict->linesize[1] + x] = 128 + y + i * 2;
            pict->data[2][y * pict->linesize[2] + x] = 64 + x + i * 5;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const Render& obj )
{
    os  << &obj           << '|'
        << obj.mSequence;
    return os;
}

} //namespace