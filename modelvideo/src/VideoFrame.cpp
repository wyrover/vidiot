#include "VideoFrame.h"

#include "Config.h"
#include "Constants.h"

#include "UtilInitAvcodec.h"
#include "UtilLogWxwidgets.h"

namespace model {

const pixel VideoFrame::sMinimumSize = 10; // To avoid errors in sws_scale all frames are at least 10 pixels in boht directions

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
    , mForceKeyFrame(false)
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
    , mForceKeyFrame(false)
{
}

// todo this constructor is never used?
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
    , mForceKeyFrame(false)
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
    , mForceKeyFrame(false)
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

// todo add support for rotating images in video composition

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

void VideoFrame::setForceKeyFrame(bool force)
{
    mForceKeyFrame = force;
}

bool VideoFrame::getForceKeyFrame() const
{
    return mForceKeyFrame;
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
        if ((mRegionOfInterest.GetPosition() == wxPoint(0,0) &&
            (mRegionOfInterest.GetSize() == mImage->GetSize())))
        {
            return mImage;
        }
    }
    else
    {
        ASSERT_ZERO(mBitmap)(mBitmap); // getBitmap() may use mImage() but otherwise is useless
        mImage = boost::make_shared<wxImage>(mSize, getData()[0], true);
    }
    return boost::make_shared<wxImage>(mImage->GetSubImage(mRegionOfInterest));
}

wxBitmapPtr VideoFrame::getBitmap()
{
    if (mRegionOfInterest.IsEmpty())
    {
        return wxBitmapPtr();
    }
    if (mBitmap)
    {
        if ((mRegionOfInterest.GetPosition() == wxPoint(0,0)) &&
            (mRegionOfInterest.GetSize() == mBitmap->GetSize()))
        {
// todo the wxBitmapPtr constructor is never used and thus this mBitmap reuse is only called after the route via getImage is done first?
            return mBitmap; // Typical case: bitmap generated by video decoding is returned directly.
        }
    }
    // all other cases:
    // - No decoded bitmap present (image is generated by video composition)
    // - Region of interest must be applied
    // Both these cases handled by getImage()

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
        << obj.mRegionOfInterest    << '|'
        << obj.mForceKeyFrame;
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