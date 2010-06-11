#include "FifoVideo.h"
#include "UtilLogAvcodec.h"

VideoFrame::VideoFrame(PixelFormat format, int width, int height, boost::int64_t pts, AVRational timebase, int repeat)
:   mFrame(0)
,	mWidth(width)
,   mHeight(height)
,   mPts(pts)
,   mTimeBase(timebase)
,   mRepeat(repeat)
{
    mBufferSize = avpicture_get_size(format, width, height);
    mBuffer = static_cast<boost::uint8_t*>(av_malloc(mBufferSize * sizeof(uint8_t)));

    mFrame = avcodec_alloc_frame();

    // Assign appropriate parts of buffer to image planes in mFrame
    avpicture_fill(reinterpret_cast<AVPicture*>(mFrame), mBuffer, format, width, height);
}

VideoFrame::~VideoFrame()
{
    av_free(mBuffer);
    av_free(mFrame);
}

DataPointer VideoFrame::getData() const
{
    return mFrame->data;
}

LineSizePointer VideoFrame::getLineSizes() const
{
    return mFrame->linesize;
}

int VideoFrame::getRepeat() const
{
    return mRepeat;

}

int VideoFrame::getWidth() const
{
    return mWidth;
}

int VideoFrame::getHeight() const
{
    return mHeight;
}

boost::int64_t VideoFrame::getPts() const
{
    return mPts;
}

void VideoFrame::setPts(boost::int64_t pts)
{
    mPts = pts;
}

AVRational VideoFrame::getTimeBase() const
{
    return mTimeBase;
}

double VideoFrame::getTime() const
{
    return mPts * av_q2d(mTimeBase);
}

int VideoFrame::getSizeInBytes() const
{
    return mBufferSize;
}

std::ostream& operator<< (std::ostream& os, const VideoFrame& obj)
{
    os  << &obj                 << "|" 
        << obj.getPts()         << "|" 
        << obj.getRepeat()      << "|" 
        << obj.getWidth()       << "|"
        << obj.getHeight();
    return os;
}

std::ostream& operator<< (std::ostream& os, const VideoFramePtr obj)
{
    if (obj)
    {
        os << *obj;
    }
    else
    {
        os << "0";
    }
    return os;
}
