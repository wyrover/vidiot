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
#include "Convert.h"
#include "Project.h"
#include "Properties.h"

namespace model {

static int const sMaxBufferSize = 10;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoFile::VideoFile()
:	File()
,   mDecodingVideo(false)
,   mDeliveredFrameInputPts(0)
,   mPosition(0)
{
    VAR_DEBUG(*this);
    mCodecType = AVMEDIA_TYPE_VIDEO;
}

VideoFile::VideoFile(boost::filesystem::path path)
:	File(path,sMaxBufferSize)
,   mDecodingVideo(false)
,   mDeliveredFrameInputPts(0)
,   mPosition(0)
{
    VAR_DEBUG(*this);
    mCodecType = AVMEDIA_TYPE_VIDEO;
}

VideoFile::VideoFile(const VideoFile& other)
:   File(other)
,   mDecodingVideo(false)
,   mDeliveredFrameInputPts(0)
,   mPosition(0)
{
    VAR_DEBUG(*this);
    mCodecType = AVMEDIA_TYPE_VIDEO;
}

VideoFile* VideoFile::clone()
{
    return new VideoFile(static_cast<const VideoFile&>(*this));
}

VideoFile::~VideoFile()
{
    VAR_DEBUG(this);
    stopDecodingVideo();
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

void VideoFile::moveTo(pts position)
{
    mDeliveredFrameInputPts = 0;
    mDeliveredFrame.reset();
    mPosition = position;
    File::moveTo(position); // NOTE: This uses the pts in 'project' timebase units
}

//////////////////////////////////////////////////////////////////////////
// IVIDEO
//////////////////////////////////////////////////////////////////////////

VideoFramePtr VideoFile::getNextVideo(int requestedWidth, int requestedHeight, bool alpha)
{
    startDecodingVideo();

    // 'Resample' the frame timebase
    // Determine which pts value is required. This is required to first determine
    // if the previously returned frame should be returned again
    // @todo instead of duplicating frames, nicely take the two input frames 'around' the 
    // required output pts time and 'interpolate' given these two frames time offsets with the required pts
    FrameRate videoFrameRate = FrameRate(mStream->codec->time_base.num, mStream->codec->time_base.den);
    int requiredInputPts = Convert::fromProjectFrameRate(mPosition, videoFrameRate);

    ASSERT(!mDeliveredFrame || requiredInputPts >= mDeliveredFrameInputPts)(mDeliveredFrameInputPts)(requiredInputPts);

    if (!mDeliveredFrame || mDeliveredFrameInputPts != requiredInputPts)
    {
        // Decode new frame
        VAR_DEBUG(mDeliveredFrameInputPts)(requiredInputPts);

        int frameFinished = 0;
        AVFrame* pFrame = avcodec_alloc_frame();
        ASSERT(pFrame != 0);

        boost::optional<int64_t> ptsOfFirstPacket = boost::none;

        while (!frameFinished )
        {
            PacketPtr packet = getNextPacket();
            if (!packet)
            {
                // End of file reached. Signal this with null ptr.
                mDeliveredFrame.reset();
                mDeliveredFrameInputPts = 0;
                static const std::string status("End of file");
                VAR_VIDEO(this)(status)(mPosition)(mDeliveredFrame)(requiredInputPts)(mDeliveredFrameInputPts);
                return mDeliveredFrame;
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
                ptsOfFirstPacket = static_cast<boost::optional<int64_t> >(packet->getPacket()->pts);
            }

            /** /todo handle decoders that hold multiple frames in one packet */
            int len1 = avcodec_decode_video(mCodecContext, pFrame, &frameFinished, packet->getPacket()->data, packet->getPacket()->size);

            if (packet->getPacket()->dts != AV_NOPTS_VALUE)
            {
                // First, try to use dts of last decoded input packet (see  text above)
                mDeliveredFrameInputPts = static_cast<double>(packet->getPacket()->dts);
            }
            else if (*ptsOfFirstPacket != AV_NOPTS_VALUE)
            {
                // Fallback, use pts of the first decoded packet for this frame
                mDeliveredFrameInputPts = static_cast<double>(*ptsOfFirstPacket);
            }
            else
            {
                NIY
            }

            // If there is no previous frame stored as a member, then this is the scenario of the
            // first getNextVideo directly after creating this file object, or directly after a move.
            // In both these cases: return the first available frame. Ignore pts mismatches.
            if (frameFinished && mDeliveredFrame && (mDeliveredFrameInputPts != requiredInputPts))
            {
                // A whole frame was decoded, but it does not have the correct pts value. Get another.
                frameFinished = 0;
            }
        }
        ASSERT(pFrame->repeat_pict >= 0)(pFrame->repeat_pict);
        if (pFrame->repeat_pict > 0)
        {
            NIY; // TO BE TESTED: FILES USING 'REPEAT'
        }

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

        mDeliveredFrame = boost::make_shared<VideoFrame>(format, scaledWidth, scaledHeight, mPosition, mCodecContext->time_base, pFrame->repeat_pict + 1);

        // Resample the frame size
        SwsContext* ctx = sws_getContext(
            mCodecContext->width,
            mCodecContext->height,
            mCodecContext->pix_fmt,
            scaledWidth,
            scaledHeight,
            format, SWS_FAST_BILINEAR | SWS_CPU_CAPS_MMX | SWS_CPU_CAPS_MMX2, 0, 0, 0);
        sws_scale(ctx,pFrame->data,pFrame->linesize,0,mCodecContext->height,mDeliveredFrame->getData(),mDeliveredFrame->getLineSizes());
        sws_freeContext(ctx);

        av_free(pFrame);
    }
    else
    {
        LOG_DEBUG << "Same frame again";
    }

    mPosition += mDeliveredFrame->getRepeat();

    VAR_VIDEO(this)(mPosition)(mDeliveredFrame)(requiredInputPts)(mDeliveredFrameInputPts);

    return mDeliveredFrame;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void VideoFile::startDecodingVideo()
{
    if (mDecodingVideo) return;
    startReadingPackets(); // Also causes the file to be opened resulting in initialized avcodec members for File.
    mDecodingVideo = true;

    boost::mutex::scoped_lock lock(sMutexAvcodec);

    mCodecContext = mStream->codec;
    //mVideoCodecContext->lowres = 2; For decoding only a 1/4 image

    AVCodec *videoCodec = avcodec_find_decoder(mStream->codec->codec_id);
    ASSERT(videoCodec != 0)(videoCodec);

    int result = avcodec_open(mCodecContext, videoCodec);
    ASSERT(result >= 0)(result);

    FrameRate videoFrameRate = FrameRate(mStream->codec->time_base.num, mStream->codec->time_base.den);
    int requiredInputPts = Convert::fromProjectFrameRate(mPosition, videoFrameRate);


    if (videoFrameRate != Project::current()->getProperties()->getFrameRate())
    {
        LOG_DEBUG << "Frame rate conversion required from " << videoFrameRate << " to " << Project::current()->getProperties()->getFrameRate();
    }

    VAR_DEBUG(this)(mCodecContext);
}

void VideoFile::stopDecodingVideo()
{
    VAR_DEBUG(this);
    if (mDecodingVideo)
    {
        boost::mutex::scoped_lock lock(sMutexAvcodec);
        avcodec_close(mCodecContext);
    }
    mDecodingVideo = false;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const VideoFile& obj )
{
    os << static_cast<const File&>(obj) << '|' << obj.mDecodingVideo << '|' << obj.mPosition << '|' << obj.mDeliveredFrameInputPts;
    return os;
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
