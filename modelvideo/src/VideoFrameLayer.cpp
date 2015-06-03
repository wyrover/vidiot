// Copyright 2013-2015 Eric Raijmakers.
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
#include "Convert.h"
#include "Constants.h"
#include "UtilInitAvcodec.h"
#include "UtilLogBoost.h"
#include "UtilLogWxwidgets.h"
#include "VideoCompositionParameters.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoFrameLayer::VideoFrameLayer(const wxImagePtr& image)
    : mImage(image)
    , mResultingImage(boost::none)
    , mPosition(0,0)
    , mOpacity(Constants::sOpacityMax)
    , mRotation(boost::none)
{
}

VideoFrameLayer::VideoFrameLayer(const VideoFrameLayer& other)
    : mImage()
    , mResultingImage(boost::none)
    , mPosition(other.mPosition)
    , mOpacity(Constants::sOpacityMax)
    , mRotation(other.mRotation)
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

void VideoFrameLayer::setPosition(const wxPoint& position)
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
    mOpacity = opacity;
    mResultingImage.reset();
}

int VideoFrameLayer::getOpacity() const
{
    return mOpacity;
}

void VideoFrameLayer::setRotation(boost::rational<int> rotation)
{
    if (rotation != boost::rational<int>(0,1))
    {
        mRotation.reset(rotation);
        mResultingImage.reset();
    }
}

wxImagePtr VideoFrameLayer::getImage()
{
    if (mResultingImage)
    {
        return *mResultingImage;
    }
    if (!mImage || !mImage->IsOk())
    {
        mResultingImage.reset(wxImagePtr());
    }
    else
    {
        ASSERT(mImage);
        if (!mImage->HasAlpha())
        {
            // Init alpha done as late as possible (avoid creating needlessly).
            if (mOpacity != Constants::sOpacityMax)
            {
                mImage->InitAlpha();
                memset(mImage->GetAlpha(),mOpacity,mImage->GetWidth() * mImage->GetHeight());
            }
            else if (mRotation)
            {
                mImage->InitAlpha(); // To avoid black being drawn besides the rotated image
            }
        }
        else
        {
            // Alpha already initialized.
            if (mOpacity != Constants::sOpacityMax)
            {
                unsigned char* alpha = mImage->GetAlpha();
                ASSERT_NONZERO(alpha);

                for (int x = 0; x < mImage->GetWidth() * mImage->GetHeight(); ++x)
                {
                    *alpha = static_cast<char>(static_cast<int>(*alpha) * mOpacity / Constants::sOpacityMax);
                    ++alpha;
                    // NOT: *alpha++ = static_cast<char>(static_cast<int>(*alpha) * mOpacity / Constants::sOpacityMax);
                    // Gives problems (on linux/GCC) because operand 'alpha' is used twice in the expression,
                    // see http://en.wikipedia.org/wiki/Increment_and_decrement_operators
                }
            }
            // else: Keep alpha data 'as is'
        }

        mResultingImage.reset(mImage);

        if (mRotation)
        {
            wxPoint center((*mResultingImage)->GetWidth() / 2, (*mResultingImage)->GetHeight() / 2);
            mResultingImage = boost::make_shared<wxImage>((*mResultingImage)->Rotate(Convert::degreesToRadians(*mRotation), center));
        }
    }
    return *mResultingImage;
}

void VideoFrameLayer::draw(wxGraphicsContext* gc, const VideoCompositionParameters& parameters)
{
    wxImagePtr image = getImage();
    if (image)
    {
#ifdef __WXGTK__
        // See [#185]. A bug in the underlying libraries causes pixels with
        //              an alpha value of '0' to be interpreted as 'opaque'.
        // That resulted in multiple problems:
        // - transitions starting with a flash as the image 'faded in' was
        //   shown fully opaque in the first step (instead of transparent).
        // - video frames opacity handling showing a opaque image instead
        //   of transparent.
        //
        // VideoFrameLayer::draw() contains the basic:
        // Loop over all pixels and for any fully transparent
        // pixels set the corresponding RGB value to 0 also.
        if (image->HasAlpha())
        {
            unsigned char* alpha = image->GetAlpha();
            ASSERT_NONZERO(alpha);
            unsigned char* rgb = image->GetData();
            ASSERT_NONZERO(rgb);
            for (int pixel = 0; pixel < image->GetWidth() * image->GetHeight(); ++pixel)
            {
                if (*alpha++ == 0)
                {
                    *rgb++ = 0;
                    *rgb++ = 0;
                    *rgb++ = 0;
                }
                else
                {
                    rgb += 3;
                }
            }
        }
#endif // __WXGTK__

        wxRect r(parameters.getRequiredRectangle());
        gc->DrawBitmap(
            gc->GetRenderer()->CreateBitmapFromImage(*image), // todo avoid all gdi calls in non gui threads.
            r.x + mPosition.x,
            r.y + mPosition.y,
            image->GetWidth(),
            image->GetHeight());
    }
    // else: No image or region of interest empty
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const VideoFrameLayer& obj)
{
    os  << &obj                     << '|'
        << obj.mPosition            << '|'
        << obj.mOpacity             << '|'
        << obj.mRotation            << '|'
        << obj.mImage;
    return os;
}

std::ostream& operator<<(std::ostream& os, const VideoFrameLayerPtr obj)
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
