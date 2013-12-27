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

#include "VideoFrameLayer.h"

#include "Config.h"
#include "Constants.h"
#include "UtilInitAvcodec.h"
#include "UtilLogWxwidgets.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoFrameLayer::VideoFrameLayer(wxImagePtr image)
    : mImage(image)
    , mResultingImage(boost::none)
    , mPosition(0,0)
    , mRegionOfInterest(wxPoint(0,0),image->GetSize())
    , mOpacity(Constants::sMaxOpacity)
{
}

VideoFrameLayer::VideoFrameLayer(const VideoFrameLayer& other)
    : mImage()
    , mResultingImage(boost::none)
    , mPosition(other.mPosition)
    , mRegionOfInterest(other.mRegionOfInterest)
    , mOpacity(Constants::sMaxOpacity)
{
    mImage = boost::make_shared<wxImage>(const_cast<VideoFrameLayer&>(other).mImage->Copy());
}

VideoFrameLayer* VideoFrameLayer::clone() const
{
    return new VideoFrameLayer(static_cast<const VideoFrameLayer&>(*this));
}

void VideoFrameLayer::onCloned()
{
}

VideoFrameLayer::~VideoFrameLayer()
{
}

//////////////////////////////////////////////////////////////////////////
// META DATA
//////////////////////////////////////////////////////////////////////////

void VideoFrameLayer::setPosition(wxPoint position)
{
    mPosition = position;
    mResultingImage.reset();
}

wxPoint VideoFrameLayer::getPosition() const
{
    return mPosition;
}

void VideoFrameLayer::setOpacity(int opacity)
{
    ASSERT(mImage);
    if (mImage->HasAlpha())
    {
        mImage->ClearAlpha();
    }
    mOpacity = opacity;
    mResultingImage.reset();
}

int VideoFrameLayer::getOpacity() const
{
    return mOpacity;
}

void VideoFrameLayer::setRegionOfInterest(wxRect regionOfInterest)
{
    ASSERT(mImage);
    ASSERT_MORE_THAN_EQUALS_ZERO(regionOfInterest.x);
    ASSERT_MORE_THAN_EQUALS_ZERO(regionOfInterest.y);
    ASSERT_MORE_THAN_EQUALS_ZERO(regionOfInterest.width);
    ASSERT_MORE_THAN_EQUALS_ZERO(regionOfInterest.height);
    ASSERT_LESS_THAN_EQUALS(regionOfInterest.x + regionOfInterest.width,  mImage->GetWidth());
    ASSERT_LESS_THAN_EQUALS(regionOfInterest.y + regionOfInterest.height, mImage->GetHeight());
    mRegionOfInterest = regionOfInterest;
    mResultingImage.reset();
}

wxImagePtr VideoFrameLayer::getImage()
{
    if (mResultingImage)
    {
        return *mResultingImage;
    }
    if (!mImage || mRegionOfInterest.IsEmpty())
    {
        mResultingImage.reset(wxImagePtr());
    }
    else
    {
        ASSERT(mImage);
        if (!mImage->HasAlpha() &&
            mOpacity != Constants::sMaxOpacity)
        {
            // Init alpha done as late as possible (avoid creating needlessly).
            mImage->InitAlpha();
            memset(mImage->GetAlpha(),mOpacity,mImage->GetWidth() * mImage->GetHeight());
        }
        // else: Alpha already initialized, or even modified (by transition) already.
        if (mRegionOfInterest.GetPosition() == wxPoint(0,0) &&
            mRegionOfInterest.GetSize() == mImage->GetSize())
        {
            mResultingImage.reset(mImage);
        }
        else
        {
            mResultingImage.reset(boost::make_shared<wxImage>(mImage->GetSubImage(mRegionOfInterest)));
        }
    }
    return *mResultingImage;
}

void VideoFrameLayer::draw(wxGraphicsContext* gc)
{
    wxImagePtr image = getImage();
    if (image)
    {
        gc->DrawBitmap(gc->GetRenderer()->CreateBitmapFromImage(*image),mPosition.x,mPosition.y,image->GetWidth(),image->GetHeight());
    }
    // else: No image or region of interest empty
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<< (std::ostream& os, const VideoFrameLayer& obj)
{
    os  << &obj                     << '|'
        << obj.mPosition            << '|'
        << obj.mRegionOfInterest    << '|'
        << obj.mImage;
    return os;
}

std::ostream& operator<< (std::ostream& os, const VideoFrameLayerPtr obj)
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