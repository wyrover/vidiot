#include "VideoFrame.h"
extern "C" {
#include <avformat.h>
};

#include <boost/make_shared.hpp>
#include "UtilInitAvcodec.h"

namespace model {

IMPLEMENTENUM(VideoFrameType);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoFrame::VideoFrame(VideoFrameType type, int width, int height, pts position, int repeat)
:   mFrame(0)
,   mBuffer(0)
,   mType(type)
,	mWidth(width)
,   mHeight(height)
,   mPts(position)
,   mRepeat(repeat)
{
    PixelFormat format = type == videoRGB ? PIX_FMT_RGB24 : PIX_FMT_RGBA;
    mBufferSize = avpicture_get_size(format, mWidth, mHeight);
    mBuffer = static_cast<boost::uint8_t*>(av_malloc(mBufferSize * sizeof(uint8_t)));

    mFrame = avcodec_alloc_frame();

    // Assign appropriate parts of buffer to image planes in mFrame
    avpicture_fill(reinterpret_cast<AVPicture*>(mFrame), mBuffer, format, mWidth, mHeight);
}

VideoFrame::VideoFrame(VideoFrameType type, int width, int height, pts position)
:   mFrame(0)
,   mBuffer(0)
,   mType(type)
,	mWidth(width)
,   mHeight(height)
,   mPts(position)
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

int VideoFrame::getSizeInBytes() const
{
    return mBufferSize;
}

wxBitmapPtr VideoFrame::getBitmap()
{
    return boost::make_shared<wxBitmap>(wxImage(getWidth(), getHeight(), getData()[0], true));
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

