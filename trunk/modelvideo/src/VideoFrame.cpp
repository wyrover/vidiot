// Copyright 2013 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

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

VideoFrame::VideoFrame(wxSize size)
    : mImage()
    , mSize(size)
    , mPosition(0,0)
    , mRegionOfInterest(wxPoint(0,0),size)
    , mPts(0)
    , mOpacity(Constants::sMaxOpacity)
    , mForceKeyFrame(false)
{
}

VideoFrame::VideoFrame(wxImagePtr image)
    : mImage(image)
    , mSize(image->GetSize())
    , mPosition(0,0)
    , mRegionOfInterest(wxPoint(0,0),image->GetSize())
    , mPts(0)
    , mOpacity(Constants::sMaxOpacity)
    , mForceKeyFrame(false)
{
}

VideoFrame::VideoFrame(const VideoFrame& other)
    : mImage()
    , mSize(other.mSize)
    , mPosition(0,0)
    , mRegionOfInterest(other.mRegionOfInterest)
    , mPts(other.mPts)
    , mOpacity(other.mOpacity)
    , mForceKeyFrame(false)
{
    if (other.mImage)
    {
        mImage = boost::make_shared<wxImage>(const_cast<VideoFrame&>(other).mImage->Copy());
    }
}

VideoFrame* VideoFrame::clone() const
{
    return new VideoFrame(static_cast<const VideoFrame&>(*this));
}

void VideoFrame::onCloned()
{
}

VideoFrame::~VideoFrame()
{
}

//////////////////////////////////////////////////////////////////////////
// META DATA
//////////////////////////////////////////////////////////////////////////

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
    ASSERT(mImage);
    if ((mRegionOfInterest.GetPosition() == wxPoint(0,0) &&
        (mRegionOfInterest.GetSize() == mImage->GetSize())))
    {
        return mImage;
    }
    return boost::make_shared<wxImage>(mImage->GetSubImage(mRegionOfInterest));
}

wxBitmapPtr VideoFrame::getBitmap()
{
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

} // namespace