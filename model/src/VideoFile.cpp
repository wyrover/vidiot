#include "VideoFile.h"

// Include at top, to exclude the intmax macros and use the boost versions
#undef INTMAX_C
#undef UINTMAX_C
#define CONFIG_SWSCALE_ALPHA
extern "C" {
#include <swscale.h>
#include <avformat.h>
}

#include <math.h>
#include <algorithm>
#include <boost/make_shared.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include "UtilLog.h"
#include "UtilLogAvcodec.h"
#include "Node.h"
#include "VideoFrame.h"
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
,   mPosition(0)
,   mDeliveredFrame()
,   mDeliveredFrameInputPts(0)
{
    VAR_DEBUG(*this);
}

VideoFile::VideoFile(wxFileName path)
:	File(path,sMaxBufferSize)
,   mDecodingVideo(false)
,   mPosition(0)
,   mDeliveredFrame()
,   mDeliveredFrameInputPts(0)
{
    VAR_DEBUG(*this);
}

VideoFile::VideoFile(const VideoFile& other)
:   File(other)
,   mDecodingVideo(false)
,   mPosition(0)
,   mDeliveredFrame()
,   mDeliveredFrameInputPts(0)
{
    VAR_DEBUG(*this);
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

void VideoFile::clean()
{
    VAR_DEBUG(this);

    stopDecodingVideo();

    mDeliveredFrameInputPts = 0;
    mDeliveredFrame.reset();
    mPosition = 0;

    File::clean();
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
    // \todo instead of duplicating frames, nicely take the two input frames 'around' the 
    // required output pts time and 'interpolate' given these two frames time offsets with the required pts
    FrameRate videoFrameRate = FrameRate(getCodec()->time_base.num, getCodec()->time_base.den);
    int requiredInputPts = Convert::fromProjectFrameRate(mPosition, videoFrameRate);

    ASSERT(!mDeliveredFrame || requiredInputPts >= mDeliveredFrameInputPts)(mDeliveredFrameInputPts)(requiredInputPts);

    if (!mDeliveredFrame || mDeliveredFrameInputPts != requiredInputPts)
    {
        // Decode new frame
        VAR_DEBUG(mDeliveredFrameInputPts)(requiredInputPts);

        int frameFinished = 0;
        AVFrame* pFrame = avcodec_alloc_frame();
        ASSERT_NONZERO(pFrame);

        boost::optional<pts> ptsOfFirstPacket = boost::none;

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
                ptsOfFirstPacket = static_cast<boost::optional<pts> >(packet->getPacket()->pts);
            }

            // \todo decoders that hold multiple frames in one packet
            int len1 = avcodec_decode_video(getCodec(), pFrame, &frameFinished, packet->getPacket()->data, packet->getPacket()->size);

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
                NIY;
            }

            // If there is no previous frame stored as a member, then this is the scenario of the
            // first getNextVideo directly after creating this file object, or directly after a move.
            // In both these cases: return the first available frame. Ignore pts mismatches.
            if (frameFinished && mDeliveredFrame && (mDeliveredFrameInputPts != requiredInputPts))
            {
                // A whole frame was decoded, but it does not have the correct pts value. Get another.
                frameFinished = 0;
            }

            // If there is no previous frame stored as a member,
            // use the get frame's position as starting point.
            if (!mDeliveredFrame)
            {
                mPosition = Convert::toProjectFrameRate(mDeliveredFrameInputPts, videoFrameRate);
            }
        }
        ASSERT_MORE_THAN_EQUALS_ZERO(pFrame->repeat_pict);
        if (pFrame->repeat_pict > 0)
        {
            NIY; // TO BE TESTED: FILES USING 'REPEAT'
        }

        wxSize scaledSize = Convert::sizeInBoundingBox(wxSize(getCodec()->width, getCodec()->height), wxSize(requestedWidth,requestedHeight));
        mDeliveredFrame = boost::make_shared<VideoFrame>(alpha ? videoRGBA : videoRGB, scaledSize.GetWidth(), scaledSize.GetHeight(), mPosition, pFrame->repeat_pict + 1);

        // Resample the frame size
        SwsContext* ctx = sws_getContext(
            getCodec()->width,
            getCodec()->height,
            getCodec()->pix_fmt,
            scaledSize.GetWidth(),
            scaledSize.GetHeight(),
            alpha ? PIX_FMT_RGBA : PIX_FMT_RGB24, SWS_FAST_BILINEAR | SWS_CPU_CAPS_MMX | SWS_CPU_CAPS_MMX2, 0, 0, 0);
        sws_scale(ctx,pFrame->data,pFrame->linesize,0,getCodec()->height,mDeliveredFrame->getData(),mDeliveredFrame->getLineSizes());
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
    // If the end of file is reached, a subsequent getNextVideo should not
    // trigger a new (useless) sequence of startReadingPackets, 
    // bufferPacketsThread, "bufferPacketsThread: End of file."
    // (and this, over and over again....).
    //
    // First a moveTo() is required to reset EOF.
    if (getEOF()) return;

    if (mDecodingVideo) return;

    startReadingPackets(); // Also causes the file to be opened resulting in initialized avcodec members for File.
    mDecodingVideo = true;

    boost::mutex::scoped_lock lock(sMutexAvcodec);

    //mStream->codec->lowres = 2; For decoding only a 1/4 image

    AVCodec *videoCodec = avcodec_find_decoder(getCodec()->codec_id);
    ASSERT_NONZERO(videoCodec);

    int result = avcodec_open(getCodec(), videoCodec);
    ASSERT_MORE_THAN_EQUALS_ZERO(result);

    FrameRate videoFrameRate = FrameRate(getCodec()->time_base.num, getCodec()->time_base.den);
    int requiredInputPts = Convert::fromProjectFrameRate(mPosition, videoFrameRate);


    if (videoFrameRate != Project::get().getProperties()->getFrameRate())
    {
        LOG_DEBUG << "Frame rate conversion required from " << videoFrameRate << " to " << Project::get().getProperties()->getFrameRate();
    }

    VAR_DEBUG(this)(getCodec());
}

void VideoFile::stopDecodingVideo()
{
    VAR_DEBUG(this);
    if (mDecodingVideo)
    {
        boost::mutex::scoped_lock lock(sMutexAvcodec);
        avcodec_close(getCodec());
    }
    mDecodingVideo = false;
}

//////////////////////////////////////////////////////////////////////////
// FROM FILE
//////////////////////////////////////////////////////////////////////////

void VideoFile::flush()
{
    if (mDecodingVideo)
    {
        avcodec_flush_buffers(getCodec());
    }
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

