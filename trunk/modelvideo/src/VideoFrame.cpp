#include "VideoFrame.h"

#include "Config.h"
#include "Constants.h"

#include "UtilInitAvcodec.h"
#include "UtilLogWxwidgets.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoFrame::VideoFrame(wxSize size, pts position, int repeat)
    : mFrame(0)
    , mImage()
    , mBitmap()
    , mBuffer(0)
    , mSize(size)
    , mPosition(0,0)
    , mRegionOfInterest(wxPoint(0,0),size)
    , mPts(position)
    , mRepeat(repeat)
    , mOpacity(Constants::sMaxOpacity)
    , mBufferSize(0)
{
    PixelFormat format = PIX_FMT_RGB24;
    mBufferSize = avpicture_get_size(format, mSize.GetWidth(), mSize.GetHeight());
    mBuffer = static_cast<boost::uint8_t*>(av_malloc(mBufferSize * sizeof(uint8_t)));

    mFrame = avcodec_alloc_frame();

    // Assign appropriate parts of buffer to image planes in mFrame
    avpicture_fill(reinterpret_cast<AVPicture*>(mFrame), mBuffer, format, mSize.GetWidth(), mSize.GetHeight());
}

    VideoFrame::VideoFrame(wxImagePtr image, pts position)
    : mFrame(0)
    , mImage(image)
    , mBitmap()
    , mBuffer(0)
    , mSize(image->GetSize())
    , mPosition(0,0)
    , mRegionOfInterest(wxPoint(0,0),image->GetSize())
    , mPts(position)
    , mRepeat(1)
    , mOpacity(Constants::sMaxOpacity)
    , mBufferSize(0)
    {
    }

    VideoFrame::VideoFrame(wxBitmapPtr bitmap, pts position)
    : mFrame(0)
    , mImage()
    , mBitmap(bitmap)
    , mBuffer(0)
    , mSize(bitmap->GetSize())
    , mPosition(0,0)
    , mRegionOfInterest(wxPoint(0,0),bitmap->GetSize())
    , mPts(position)
    , mRepeat(1)
    , mOpacity(Constants::sMaxOpacity)
    , mBufferSize(0)
    {
    }

VideoFrame::VideoFrame(wxSize size, pts position)
    : mFrame(0)
    , mImage()
    , mBitmap()
    , mBuffer(0)
    , mSize(size)
    , mPosition(0,0)
    , mRegionOfInterest(wxPoint(0,0),size)
    , mPts(position)
    , mRepeat(1)
    , mOpacity(Constants::sMaxOpacity)
    , mBufferSize(0)
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

wxSize VideoFrame::getSize() const
{
    return mSize;
}

void VideoFrame::setPosition(wxPoint position)
{
    mPosition = position;
}

wxPoint VideoFrame::getPosition() const
{
    return mPosition;
}

int VideoFrame::getOpacity() const
{
    return mOpacity;
}

void VideoFrame::setOpacity(int opacity)
{
    mOpacity = opacity;
}

void VideoFrame::setRegionOfInterest(wxRect regionOfInterest)
{
    ASSERT_MORE_THAN_EQUALS_ZERO(regionOfInterest.x);
    ASSERT_MORE_THAN_EQUALS_ZERO(regionOfInterest.y);
    ASSERT_MORE_THAN_EQUALS_ZERO(regionOfInterest.width);
    ASSERT_MORE_THAN_EQUALS_ZERO(regionOfInterest.height);
    ASSERT_LESS_THAN_EQUALS(regionOfInterest.x + regionOfInterest.width,  mSize.x);
    ASSERT_LESS_THAN_EQUALS(regionOfInterest.y + regionOfInterest.height, mSize.y);
    mRegionOfInterest = regionOfInterest;
}

wxRect VideoFrame::getRegionOfInterest() const
{
    return mRegionOfInterest;
}

pts VideoFrame::getPts() const
{
    return mPts;
}

void VideoFrame::setPts(pts position)
{
    mPts = position;
}

wxImagePtr VideoFrame::getImage()
{
    if (mRegionOfInterest.IsEmpty())
    {
        return wxImagePtr();
    }
    if (mImage)
    {
        ASSERT_EQUALS(mRegionOfInterest.GetPosition(),wxPoint(0,0));  // These checks are done to ensure that no scenarios exist in which mImage is set and the region of
        ASSERT_EQUALS(mRegionOfInterest.GetSize(),mImage->GetSize()); // interest is changed. That will not work since we here do not take into account region of interest..
        return mImage;
    }
    ASSERT_ZERO(mBitmap)(mBitmap); // getBitmap() may use mImage() but otherwise is useless
    wxImage tmp(mSize, getData()[0], true);
    return boost::make_shared<wxImage>(tmp.GetSubImage(mRegionOfInterest));
}

wxBitmapPtr VideoFrame::getBitmap()
{
    if (mBitmap)
    {
        ASSERT_EQUALS(mRegionOfInterest.GetPosition(),wxPoint(0,0));   // These checks are done to ensure that no scenarios exist in which mBitmap is set and the region of
        ASSERT_EQUALS(mRegionOfInterest.GetSize(),mBitmap->GetSize()); // interest is changed. That will not work since we here do not take into account region of interest..
        return mBitmap;
    }
    wxImagePtr image(getImage());
    if (!image)
    {
        return wxBitmapPtr();
    }
    return boost::make_shared<wxBitmap>(*image);
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<< (std::ostream& os, const VideoFrame& obj)
{
    os  << &obj                     << '|'
        << obj.mPts                 << '|'
        << obj.mRepeat              << '|'
        << obj.mSize                << '|'
        << obj.mPosition            << '|'
        << obj.mOpacity             << '|'
        << obj.mRegionOfInterest;
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