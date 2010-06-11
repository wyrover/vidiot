#include "VideoFile.h"

#define CONFIG_SWSCALE_ALPHA
extern "C" {
#include <swscale.h>
}

#include <math.h>
#include <algorithm>
#include <boost/make_shared.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include "UtilLog.h"
#include "UtilLogAvcodec.h"
#include "AProjectViewNode.h"

namespace model {

static int const sMaxBufferSize = 10;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoFile::VideoFile()
:	File()
,   mDecodingVideo(false)
,   mVideoAspectRatio(0)
{ 
    mCodecType = CODEC_TYPE_VIDEO;

    VAR_DEBUG(this);
}

VideoFile::VideoFile(boost::filesystem::path path)
:	File(path,sMaxBufferSize)
,   mDecodingVideo(false)
,   mVideoAspectRatio(0)
{ 
    mCodecType = CODEC_TYPE_VIDEO;

    VAR_DEBUG(this);
}

VideoFile::~VideoFile()
{
    VAR_DEBUG(this);
    stopDecodingVideo();
}

void VideoFile::startDecodingVideo()
{
    if (mDecodingVideo) return;
    startReadingPackets(); // Also causes the file to be opened resulting in initialized avcodec members for File.
    mDecodingVideo = true;

    boost::mutex::scoped_lock lock(sMutexAvcodec);

    mCodecContext = mStream->codec;
    //mVideoCodecContext->lowres = 2; For decoding only a 1/4 image

    mVideoAspectRatio = 0;
    if (mStream->codec->sample_aspect_ratio.num != 0) 
    {
        mVideoAspectRatio = av_q2d(mStream->codec->sample_aspect_ratio) * mCodecContext->width / mCodecContext->height;
    }

    AVCodec *videoCodec = avcodec_find_decoder(mStream->codec->codec_id);
    ASSERT(videoCodec != 0)(videoCodec);

    int result = avcodec_open(mCodecContext, videoCodec);
    ASSERT(result >= 0)(result);

    VAR_INFO(mCodecContext)(mVideoAspectRatio);
}

void VideoFile::stopDecodingVideo()
{
    if (mDecodingVideo)
    {
        boost::mutex::scoped_lock lock(sMutexAvcodec);
        avcodec_close(mCodecContext);
    }
    mDecodingVideo = false;
}

VideoFramePtr VideoFile::getNextVideo(int requestedWidth, int requestedHeight, bool alpha)
{
    startDecodingVideo();

    int frameFinished = 0;
    int repeat = 0;
    int64_t pts = 0;
    AVFrame* pFrame = avcodec_alloc_frame();
    ASSERT(pFrame != 0);

    boost::optional<int64_t> ptsOfFirstPacket = boost::none;

    while (!frameFinished) 
    {
        PacketPtr packet = getNextPacket();
        if (!packet)
        {
            LOG_DEBUG << "End of file";
            // End of file reached. Signal this with null ptr.
            return VideoFramePtr();
        }

        // From http://dranger.com/ffmpeg/tutorial05.html:
        // When we get a packet from av_read_frame(), it will contain the PTS and DTS values for the information inside that packet. 
        // But what we really want is the PTS of our newly decoded raw frame, so we know when to display it. However, the frame we 
        // get from avcodec_decode_video() gives us an AVFrame, which doesn't contain a useful PTS value.
        // (Warning: AVFrame does contain a pts variable, but this will not always contain what we want when we get a frame.) 
        // However, ffmpeg reorders the packets so that the DTS of the packet being processed by avcodec_decode_video() will always 
        // be the same as the PTS of the frame it returns. But, another warning: we won't always get this information, either.
        //
        // Not to worry, because there's another way to find out the PTS of a frame, and we can have our program reorder the packets 
        // by itself. We save the PTS of the first packet of a frame: this will be the PTS of the finished frame. So when the stream 
        // doesn't give us a DTS, we just use this saved PTS. 
        // Of course, even then, we might not get a proper pts. We'll deal with that later. 
        if (!ptsOfFirstPacket)
        {
            ptsOfFirstPacket = static_cast<boost::optional<int64_t>>(packet->getPacket()->pts);
        }

        /** /todo handle decoders that hold multiple frames in one packet */
        int len1 = avcodec_decode_video(mCodecContext, pFrame, &frameFinished, packet->getPacket()->data, packet->getPacket()->size);

        if (packet->getPacket()->dts != AV_NOPTS_VALUE) 
        {
            // First, try to use dts of last decoded input packet (see  text above)
            pts = static_cast<double>(packet->getPacket()->dts);
        } 
        else if (*ptsOfFirstPacket != AV_NOPTS_VALUE) 
        {
            // Fallback, use pts of the first decoded packet for this frame
            pts = static_cast<double>(*ptsOfFirstPacket);
        }
        else
        {
            NIY
        }
    }
    repeat = pFrame->repeat_pict;

    static const int sMinimumSize = 10; // Used to avoid crashes in sws_scale (too small bitmaps)
    double w = std::max(sMinimumSize, requestedWidth);
    double h = std::max(sMinimumSize, requestedHeight);
    double scalingW = w / static_cast<double>(mCodecContext->width);
    double scalingH = h / static_cast<double>(mCodecContext->height);
    double scaling  = std::min(scalingW, scalingH);
    int scaledWidth  = static_cast<int>(floor(scaling * mCodecContext->width));
    int scaledHeight = static_cast<int>(floor(scaling * mCodecContext->height));

    PixelFormat format = PIX_FMT_RGBA;
    if (!alpha)
    {
        format = PIX_FMT_RGB24;
    }

    VideoFramePtr videoFrame = boost::make_shared<VideoFrame>(format, scaledWidth, scaledHeight, pts, mCodecContext->time_base, repeat);

    SwsContext* ctx2 = sws_getContext(
        mCodecContext->width, 
        mCodecContext->height, 
        mCodecContext->pix_fmt, 
        scaledWidth, 
        scaledHeight, 
        format, SWS_FAST_BILINEAR | SWS_CPU_CAPS_MMX | SWS_CPU_CAPS_MMX2, 0, 0, 0);

    sws_scale(ctx2,pFrame->data,pFrame->linesize,0,mCodecContext->height,videoFrame->getData(),videoFrame->getLineSizes());

    /** /todo video resampling. Especially needed for casio shots. */

    sws_freeContext(ctx2);
    av_free(pFrame);

    VAR_VIDEO(videoFrame);
    return videoFrame;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void VideoFile::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<File>(*this);
    ar & boost::serialization::base_object<IVideo>(*this);
}
template void VideoFile::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void VideoFile::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace
