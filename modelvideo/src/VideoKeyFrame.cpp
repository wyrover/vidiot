// Copyright 2015-2016 Eric Raijmakers.
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

#include "VideoKeyFrame.h"

#include "Config.h"
#include "Convert.h"
#include "EmptyFrame.h"
#include "Properties.h"
#include "StatusBar.h"
#include "UtilSerializeBoost.h"
#include "UtilSerializeWxwidgets.h"
#include "VideoCompositionParameters.h"
#include "VideoFile.h"
#include "VideoFrameLayer.h"
#include "VideoSkipFrame.h"

namespace model {


//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

const int VideoKeyFrame::sOpacityMin{ wxIMAGE_ALPHA_TRANSPARENT };
const int VideoKeyFrame::sOpacityMax{ wxIMAGE_ALPHA_OPAQUE };
const rational64 VideoKeyFrame::sScalingMin{ 1,100 }; // 0.01
const rational64 VideoKeyFrame::sScalingMax{ 100,1 }; // 100

VideoKeyFrame::VideoKeyFrame()
    : KeyFrame{ false }
    , mInputSize{ 0,0 }
    , mOpacity{ sOpacityMax }
    , mScaling{}
    , mScalingFactor{ 1 }
    , mRotation{ 0 }
    , mRotationPositionOffset{ 0, 0 }
    , mAlignment{ VideoAlignmentCenter }
    , mPosition{ 0, 0 }
{
    VAR_DEBUG(*this);
}

VideoKeyFrame::VideoKeyFrame(const wxSize& size)
    : KeyFrame{ false }
    , mInputSize{ 0,0 }
    , mOpacity{ sOpacityMax }
    , mScaling{}
    , mScalingFactor{ 1 }
    , mRotation{ 0 }
    , mRotationPositionOffset{ 0, 0 }
    , mAlignment{ VideoAlignmentCenter }
    , mPosition{ 0, 0 }
{
    mInputSize = size;
    mScaling = Config::get().ReadEnum<VideoScaling>(Config::sPathVideoDefaultScaling);
    mAlignment = Config::get().ReadEnum<VideoAlignment>(Config::sPathVideoDefaultAlignment);
    updateAutomatedScaling();
    updateAutomatedPositioning();
    VAR_DEBUG(*this);
}

VideoKeyFrame::VideoKeyFrame(VideoKeyFramePtr before, VideoKeyFramePtr after, pts positionBefore, pts position, pts positionAfter)
    : KeyFrame{ false }
{
    ASSERT_NONZERO(before);
    ASSERT_NONZERO(after);
    ASSERT_EQUALS(before->getInputSize(), after->getInputSize());
    ASSERT_LESS_THAN(positionBefore, position);
    ASSERT_LESS_THAN(position, positionAfter);
    rational64 factor{ position - positionBefore, positionAfter - positionBefore };
    ASSERT_MORE_THAN_EQUALS_ZERO(factor);
    ASSERT_LESS_THAN(factor, 1);

    mInputSize = before->getInputSize();
    mOpacity = before->getOpacity() + boost::rational_cast<int>(factor * (rational64(after->getOpacity() - before->getOpacity())));
    mScaling = model::VideoScalingCustom;
    mScalingFactor = before->getScalingFactor() + (factor * (rational64(after->getScalingFactor() - before->getScalingFactor())));
    ASSERT_MORE_THAN_ZERO(mScalingFactor);
    mRotation = before->getRotation() + (factor * (rational64(after->getRotation() - before->getRotation())));
    mAlignment = model::VideoAlignmentCustom;
    mPosition.x = before->getPosition().x + boost::rational_cast<int>(factor * (rational64(after->getPosition().x - before->getPosition().x)));
    mPosition.y = before->getPosition().y + boost::rational_cast<int>(factor * (rational64(after->getPosition().y - before->getPosition().y)));
    mCropTop = before->getCropTop() + boost::rational_cast<int>(factor * (rational64(after->getCropTop() - before->getCropTop())));
    mCropBottom = before->getCropBottom() + boost::rational_cast<int>(factor * (rational64(after->getCropBottom() - before->getCropBottom())));
    mCropLeft = before->getCropLeft() + boost::rational_cast<int>(factor * (rational64(after->getCropLeft() - before->getCropLeft())));
    mCropRight = before->getCropRight() + boost::rational_cast<int>(factor * (rational64(after->getCropRight() - before->getCropRight())));
    updateAutomatedPositioning(); //Update mRotationPositionOffset
}

VideoKeyFrame::VideoKeyFrame(const VideoKeyFrame& other)
    : KeyFrame{ other }
    , mInputSize{ other.mInputSize }
    , mOpacity{ other.mOpacity }
    , mScaling{ other.mScaling }
    , mScalingFactor{ other.mScalingFactor }
    , mRotation{ other.mRotation }
    , mRotationPositionOffset{ other.mRotationPositionOffset }
    , mAlignment{ other.mAlignment }
    , mPosition{ other.mPosition }
    , mCropTop{ other.mCropTop }
    , mCropBottom{ other.mCropBottom }
    , mCropLeft{ other.mCropLeft }
    , mCropRight{ other.mCropRight }
{
    VAR_DEBUG(other)(*this);
}

VideoKeyFrame* VideoKeyFrame::clone() const
{
    return new VideoKeyFrame(static_cast<const VideoKeyFrame&>(*this));
}

VideoKeyFrame::~VideoKeyFrame()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

wxRect VideoKeyFrame::getCroppedRect() const
{
    int x{ 0 };
    int y{ 0 };
    int w{ mInputSize.x };
    int h{ mInputSize.y };
    x += mCropLeft;
    y += mCropTop;
    w -= mCropLeft + mCropRight;
    h -= mCropTop + mCropBottom;
    ASSERT_MORE_THAN_ZERO(w); // Required for computations. This may never be
    ASSERT_MORE_THAN_ZERO(h); // zero, since it might be used as a denominator.
    return wxRect{ x,y,w,h };
}

wxSize VideoKeyFrame::getOutputSize() const
{
    wxSize croppedSize{ getCroppedRect().GetSize() };
    wxSize scaledsize = Convert::scale(croppedSize, getScalingFactor());
    return scaledsize;
}

int VideoKeyFrame::getOpacity() const
{
    return mOpacity;
}

VideoScaling VideoKeyFrame::getScaling() const
{
    return mScaling;
}

rational64 VideoKeyFrame::getScalingFactor() const
{
    return mScalingFactor;
}

rational64 VideoKeyFrame::getRotation() const
{
    return mRotation;
}

wxPoint VideoKeyFrame::getRotationPositionOffset() const
{
    return mRotationPositionOffset;
}

VideoAlignment VideoKeyFrame::getAlignment() const
{
    return mAlignment;
}

wxPoint VideoKeyFrame::getPosition() const
{
    return mPosition;
}

int VideoKeyFrame::getCropTop()
{
    return mCropTop;
}

int VideoKeyFrame::getCropBottom()
{
    return mCropBottom;
}

int VideoKeyFrame::getCropLeft()
{
    return mCropLeft;
}

int VideoKeyFrame::getCropRight()
{
    return mCropRight;
}

wxPoint VideoKeyFrame::getMinPosition()
{
    wxSize boundingBox = getBoundingBox();
    return wxPoint(-boundingBox.x, -boundingBox.y);
}

wxPoint VideoKeyFrame::getMaxPosition()
{
    wxSize targetSize = Properties::get().getVideoSize();
    wxSize boundingBox = getBoundingBox();
    int maxX = std::max(boundingBox.x, targetSize.x);
    int maxY = std::max(boundingBox.y, targetSize.y);
    return wxPoint(maxX, maxY) + mRotationPositionOffset;
}

void VideoKeyFrame::setOpacity(int opacity)
{
    ASSERT(!isInterpolated())(*this);
    if (mOpacity != opacity)
    {
        ASSERT_MORE_THAN_EQUALS(opacity, sOpacityMin);
        ASSERT_LESS_THAN_EQUALS(opacity, sOpacityMax);
        mOpacity = opacity;
    }
}

void VideoKeyFrame::setCropTop(int crop)
{
    ASSERT(!isInterpolated())(*this);
    if (mCropTop != crop)
    {
        ASSERT_MORE_THAN_EQUALS(crop, sCropMin);
        ASSERT_LESS_THAN_EQUALS(crop, sCropMax);
        mCropTop = crop;
    }
}

void VideoKeyFrame::setCropBottom(int crop)
{
    ASSERT(!isInterpolated())(*this);
    if (mCropBottom != crop)
    {
        ASSERT_MORE_THAN_EQUALS(crop, sCropMin);
        ASSERT_LESS_THAN_EQUALS(crop, sCropMax);
        mCropBottom = crop;
    }
}

void VideoKeyFrame::setCropLeft(int crop)
{
    ASSERT(!isInterpolated())(*this);
    if (mCropLeft != crop)
    {
        ASSERT_MORE_THAN_EQUALS(crop, sCropMin);
        ASSERT_LESS_THAN_EQUALS(crop, sCropMax);
        mCropLeft = crop;
    }
}

void VideoKeyFrame::setCropRight(int crop)
{
    ASSERT(!isInterpolated())(*this);
    if (mCropRight != crop)
    {
        ASSERT_MORE_THAN_EQUALS(crop, sCropMin);
        ASSERT_LESS_THAN_EQUALS(crop, sCropMax);
        mCropRight = crop;
    }
}

void VideoKeyFrame::setScaling(const VideoScaling& scaling, const boost::optional<rational64 >& factor)
{
    ASSERT(!isInterpolated())(*this);
                        
    mScaling = scaling;
    if (factor)
    {
        wxSize outputSize{ getOutputSize() };
        ASSERT_MORE_THAN_ZERO(*factor)(*factor);
        unsigned int w{ boost::rational_cast<unsigned int>(outputSize.GetWidth() * *factor) };
        unsigned int h{ boost::rational_cast<unsigned int>(outputSize.GetHeight() * *factor) };
        if (w == 0)
        {
            gui::StatusBar::get().timedInfoText(_("Width becomes 0."));
        }
        else if (h == 0)
        {
            gui::StatusBar::get().timedInfoText(_("Height becomes 0."));
        }
        else if (av_image_check_size(w, h, 0, 0) < 0)
        {
            gui::StatusBar::get().timedInfoText(_("Image becomes too large."));
        }
        else
        {
            mScalingFactor = *factor;
        }
    }

    updateAutomatedScaling();
    updateAutomatedPositioning();
}

void VideoKeyFrame::setRotation(const rational64& rotation)
{
    ASSERT(!isInterpolated())(*this);

    mRotation = rotation;

    updateAutomatedPositioning();
}

void VideoKeyFrame::setRotationPositionOffset(wxPoint position)
{
    ASSERT(!isInterpolated())(*this);

    mRotationPositionOffset = position;
}

void VideoKeyFrame::setAlignment(const VideoAlignment& alignment)
{
    ASSERT(!isInterpolated())(*this);

    mAlignment = alignment;

    updateAutomatedPositioning();
}

void VideoKeyFrame::setPosition(const wxPoint& position)
{
    ASSERT(!isInterpolated())(*this);

    mPosition = position;

    updateAutomatedPositioning();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

wxSize VideoKeyFrame::getBoundingBox()
{
    wxSize outputSize{ getOutputSize() };
    if (mRotation == rational64(0))
    {
        return outputSize;
    }

    int boundingBoxHeight = abs(outputSize.x * sin(Convert::degreesToRadians(mRotation))) + abs(outputSize.y * cos(Convert::degreesToRadians(mRotation)));
    int boundingBoxWidth = abs(outputSize.x * cos(Convert::degreesToRadians(mRotation))) + abs(outputSize.y * sin(Convert::degreesToRadians(mRotation)));
    return wxSize(boundingBoxWidth, boundingBoxHeight);
}

void VideoKeyFrame::updateAutomatedScaling()
{
    wxSize croppedSize{ getCroppedRect().GetSize() };
    wxSize outputSize{ getOutputSize() };
    wxSize boundingBoxSize{ getBoundingBox() };
    wxSize targetSize{ Properties::get().getVideoSize() };

    switch (mScaling)
    {
    case VideoScalingFitToFill:
    {
        rational64 scalingfactor;
        Convert::sizeInBoundingBox(croppedSize, targetSize, mScalingFactor, true); // The true ensures that the bounding box is filled
        break;
    }
    case VideoScalingFitAll:
    {
        rational64 scalingfactor;
        Convert::sizeInBoundingBox(croppedSize, targetSize, mScalingFactor, false); // The false ensures that the entire video contents is shown (with black bars to fill the bounding box)
        break;
    }
    case VideoScalingNone:
    {
        mScalingFactor = 1;
        break;
    }
    case VideoScalingHalf:
    {
        mScalingFactor = rational64(1, 2);
        break;
    }
    case VideoScalingThird:
    {
        mScalingFactor = rational64(1, 3);
        break;
    }
    case VideoScalingFourth:
    {
        mScalingFactor = rational64(1, 4);
        break;
    }
    case VideoScalingCustom:
    default:
        // Do not automatically determine scaling factor
        break;
    }

    // Ensure that automated scaling never causes the scaling to exceed on of the scaling bounds
    if (mScalingFactor > VideoKeyFrame::sScalingMax)
    {
        mScalingFactor = VideoKeyFrame::sScalingMax;
    }
    if (mScalingFactor < VideoKeyFrame::sScalingMin)
    {
        mScalingFactor = VideoKeyFrame::sScalingMin;
    }
    ASSERT_MORE_THAN_ZERO(mScalingFactor);
}

void VideoKeyFrame::updateAutomatedPositioning()
{
    wxSize outputSize{ getOutputSize() };
    wxSize targetSize = Properties::get().getVideoSize();
    wxSize boundingBox = getBoundingBox();

    mRotationPositionOffset = wxPoint((boundingBox.x - outputSize.x) / 2, (boundingBox.y - outputSize.y) / 2);

    switch (mAlignment)
    {
        case VideoAlignmentCenter:
        {
            mPosition.x = (targetSize.GetWidth() - outputSize.GetWidth()) / 2;
            mPosition.y = (targetSize.GetHeight() - outputSize.GetHeight()) / 2;
            break;
        }
        case VideoAlignmentCenterHorizontal:
        {
            mPosition.x = (targetSize.GetWidth() - outputSize.GetWidth()) / 2;
            break;
        }
        case VideoAlignmentCenterVertical:
        {
            mPosition.y = (targetSize.GetHeight() - outputSize.GetHeight()) / 2;
            break;
        }
        case model::VideoAlignmentTopLeft:
        {
            mPosition.x = 0;
            mPosition.y = 0;
            break;
        }
        case model::VideoAlignmentTop:
        {
            mPosition.y = 0;
            break;
        }
        case model::VideoAlignmentTopCenter:
        {
            mPosition.x = (targetSize.GetWidth() - outputSize.GetWidth()) / 2;
            mPosition.y = 0;
            break;
        }
        case model::VideoAlignmentTopRight:
        {
            mPosition.y = 0;
            mPosition.x = targetSize.GetWidth() - outputSize.GetWidth();
            break;
        }
        case model::VideoAlignmentRight:
        {
            mPosition.x = targetSize.GetWidth() - outputSize.GetWidth();
            break;
        }
        case model::VideoAlignmentRightCenter:
        {
            mPosition.x = targetSize.GetWidth() - outputSize.GetWidth();
            mPosition.y = (targetSize.GetHeight() - outputSize.GetHeight()) / 2;
            break;
        }
        case model::VideoAlignmentBottomRight:
        {
            mPosition.x = targetSize.GetWidth() - outputSize.GetWidth();
            mPosition.y = targetSize.GetHeight() - outputSize.GetHeight();
            break;
        }
        case model::VideoAlignmentBottom:
        {
            mPosition.y = targetSize.GetHeight() - outputSize.GetHeight();
            break;
        }
        case model::VideoAlignmentBottomCenter:
        {
            mPosition.x = (targetSize.GetWidth() - outputSize.GetWidth()) / 2;
            mPosition.y = targetSize.GetHeight() - outputSize.GetHeight();
            break;
        }
        case model::VideoAlignmentBottomLeft:
        {
            mPosition.x = 0;
            mPosition.y = targetSize.GetHeight() - outputSize.GetHeight();
            break;
        }
        case model::VideoAlignmentLeft:
        {
            mPosition.x = 0;
            break;
        }
        case model::VideoAlignmentLeftCenter:
        {
            mPosition.x = 0;
            mPosition.y = (targetSize.GetHeight() - outputSize.GetHeight()) / 2;
            break;
        }
        case VideoAlignmentCustom:
            // Use current offsets
        default:
            break;
    }

    if (mPosition.x < getMinPosition().x) { mPosition.x = getMinPosition().x; } // Avoid setting illegal values. This can happen if events for new boundaries
    if (mPosition.y < getMinPosition().y) { mPosition.y = getMinPosition().y; } // have not been received yet, by the GUI part which triggers these changes.
    if (mPosition.x > getMaxPosition().x) { mPosition.x = getMaxPosition().x; } // This can also happen when first setting the clip to a certain position, and
    if (mPosition.y > getMaxPosition().y) { mPosition.y = getMaxPosition().y; } // then changing the scaling.
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const VideoKeyFrame& obj)
{
    os << static_cast<const KeyFrame&>(obj) << '|'
        << std::setw(4) << obj.mInputSize << '|'
        << std::setw(2) << std::hex << obj.mOpacity << std::dec << '|'
        << obj.mScaling << '|'
        << obj.mScalingFactor << '|'
        << obj.mRotation << '|'
        << obj.mRotationPositionOffset << '|'
        << obj.mAlignment << '|'
        << obj.mPosition << '|'
        << obj.mCropTop << '|'
        << obj.mCropBottom << '|'
        << obj.mCropLeft << '|'
        << obj.mCropRight;
        return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void VideoKeyFrame::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(KeyFrame);
        ar & BOOST_SERIALIZATION_NVP(mInputSize);
        ar & BOOST_SERIALIZATION_NVP(mOpacity);
        ar & BOOST_SERIALIZATION_NVP(mScaling);
        ar & BOOST_SERIALIZATION_NVP(mScalingFactor);
        ar & BOOST_SERIALIZATION_NVP(mRotation);
        ar & BOOST_SERIALIZATION_NVP(mRotationPositionOffset);
        ar & BOOST_SERIALIZATION_NVP(mAlignment);
        ar & BOOST_SERIALIZATION_NVP(mPosition);
        if (version >= 2)
        {
            ar & BOOST_SERIALIZATION_NVP(mCropTop);
            ar & BOOST_SERIALIZATION_NVP(mCropBottom);
            ar & BOOST_SERIALIZATION_NVP(mCropLeft);
            ar & BOOST_SERIALIZATION_NVP(mCropRight);
        }
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void VideoKeyFrame::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void VideoKeyFrame::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::VideoKeyFrame)
