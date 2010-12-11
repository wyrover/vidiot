#include "VideoFrame.h"
#include "UtilLogAvcodec.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoFrame::VideoFrame(PixelFormat format, int width, int height, pts position, AVRational timebase, int repeat)
:   mFrame(0)
,   mBuffer(0)
,   mFormat(format)
,	mWidth(width)
,   mHeight(height)
,   mPts(position)
,   mTimeBase(timebase)
,   mRepeat(repeat)
{
    mBufferSize = avpicture_get_size(mFormat, mWidth, mHeight);
    mBuffer = static_cast<boost::uint8_t*>(av_malloc(mBufferSize * sizeof(uint8_t)));

    mFrame = avcodec_alloc_frame();

    // Assign appropriate parts of buffer to image planes in mFrame
    avpicture_fill(reinterpret_cast<AVPicture*>(mFrame), mBuffer, mFormat, mWidth, mHeight);
}

VideoFrame::VideoFrame(PixelFormat format, int width, int height, pts position, AVRational timebase)
:   mFrame(0)
,   mBuffer(0)
,   mFormat(format)
,	mWidth(width)
,   mHeight(height)
,   mPts(position)
,   mTimeBase(timebase)
,   mRepeat(1)
{
}

VideoFrame::~VideoFrame()
{

    if (mBuffer)
    {
        av_free(mBuffer);
    }
    if (mFrame)
    {
        av_free(mFrame);
    }
}

//////////////////////////////////////////////////////////////////////////
// META DATA
//////////////////////////////////////////////////////////////////////////

int VideoFrame::getRepeat() const
{
    return mRepeat;

}

void VideoFrame::setRepeat(int repeat)
{
    mRepeat = repeat;
}

int VideoFrame::getWidth() const
{
    return mWidth;
}

int VideoFrame::getHeight() const
{
    return mHeight;
}

pts VideoFrame::getPts() const
{
    return mPts;
}

void VideoFrame::setPts(pts position)
{
    mPts = position;
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

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

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
//////////////////////////////////////////////////////////////////////////
// DATA ACCESS
//////////////////////////////////////////////////////////////////////////

DataPointer VideoFrame::getData()
{
    return mFrame->data;
}

LineSizePointer VideoFrame::getLineSizes() const
{
    return mFrame->linesize;
}

} // namespace

