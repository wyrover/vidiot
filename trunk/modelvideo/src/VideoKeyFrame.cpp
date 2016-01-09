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
    , mAlignment{}
    , mPosition{ 0, 0 }
{
    VAR_DEBUG(*this);
}

VideoKeyFrame::VideoKeyFrame(const wxSize& size)
    : KeyFrame{ false }
{
    VAR_DEBUG(*this);
    mInputSize = size;
    mScaling = Config::ReadEnum<VideoScaling>(Config::sPathVideoDefaultScaling);
    mAlignment = Config::ReadEnum<VideoAlignment>(Config::sPathVideoDefaultAlignment);
    updateAutomatedScaling();
    updateAutomatedPositioning();
}

VideoKeyFrame::VideoKeyFrame(VideoKeyFramePtr before, VideoKeyFramePtr after, pts positionBefore, pts position, pts positionAfter)
    : KeyFrame{ false }
{
    ASSERT_NONZERO(before);
    ASSERT_NONZERO(after);
    ASSERT_EQUALS(before->getSize(), after->getSize());
    ASSERT_LESS_THAN(positionBefore, position);
    ASSERT_LESS_THAN(position, positionAfter);
    rational64 factor{ position - positionBefore, positionAfter - positionBefore };
    ASSERT_MORE_THAN_EQUALS_ZERO(factor);
    ASSERT_LESS_THAN(factor, 1);

    mInputSize = before->getSize();
    mOpacity = before->getOpacity() + boost::rational_cast<int>(factor * (rational64(after->getOpacity() - before->getOpacity())));
    mScaling = model::VideoScalingCustom;
    mScalingFactor = before->getScalingFactor() + (factor * (rational64(after->getScalingFactor() - before->getScalingFactor())));
    ASSERT_MORE_THAN_ZERO(mScalingFactor);
    mRotation = before->getRotation() + (factor * (rational64(after->getRotation() - before->getRotation())));
    mAlignment = model::VideoAlignmentCustom;
    mPosition.x = before->getPosition().x + boost::rational_cast<int>(factor * (rational64(after->getPosition().x - before->getPosition().x)));
    mPosition.y = before->getPosition().y + boost::rational_cast<int>(factor * (rational64(after->getPosition().y - before->getPosition().y)));
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

wxPoint VideoKeyFrame::getMinPosition()
{
    wxSize boundingBox = getBoundingBox();
    return wxPoint(-boundingBox.x, -boundingBox.y);
}

wxPoint VideoKeyFrame::getMaxPosition()
{
    wxSize outputsize = Properties::get().getVideoSize();
    wxSize boundingBox = getBoundingBox();
    int maxX = std::max(boundingBox.x, outputsize.x);
    int maxY = std::max(boundingBox.y, outputsize.y);
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

void VideoKeyFrame::setScaling(const VideoScaling& scaling, const boost::optional<rational64 >& factor)
{
    ASSERT(!isInterpolated())(*this);
    VideoScaling oldScaling = mScaling;
    rational64 oldScalingFactor = mScalingFactor;
    wxPoint oldPosition = mPosition;
    wxPoint oldMinPosition = getMinPosition();
    wxPoint oldMaxPosition = getMaxPosition();

    mScaling = scaling;
    if (factor)
    {
        ASSERT_MORE_THAN_ZERO(*factor)(*factor);
        unsigned int w{ boost::rational_cast<unsigned int>(mInputSize.GetWidth() * *factor) };
        unsigned int h{ boost::rational_cast<unsigned int>(mInputSize.GetHeight() * *factor) };
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
    rational64 oldRotation = mRotation;
    wxPoint oldPosition = mPosition;
    wxPoint oldMinPosition = getMinPosition();
    wxPoint oldMaxPosition = getMaxPosition();

    mRotation = rotation;

    updateAutomatedPositioning();
}

void VideoKeyFrame::setRotationPositionOffset(wxPoint position)
{
    mRotationPositionOffset = position;
}

void VideoKeyFrame::setAlignment(const VideoAlignment& alignment)
{
    ASSERT(!isInterpolated())(*this);
    VideoAlignment oldAlignment = mAlignment;
    wxPoint oldPosition = mPosition;

    mAlignment = alignment;

    updateAutomatedPositioning();
}

void VideoKeyFrame::setPosition(const wxPoint& position)
{
    ASSERT(!isInterpolated())(*this);
    VAR_INFO(position);
    wxPoint oldPosition = mPosition;
    mPosition = position;

    updateAutomatedPositioning();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

wxSize VideoKeyFrame::getBoundingBox()
{
    ASSERT_DIFFERS(mInputSize, wxSize(0, 0));
    wxSize scaledsize = Convert::scale(mInputSize, getScalingFactor());
    if (mRotation == rational64(0))
    {
        return scaledsize;
    }

    int boundingBoxHeight = abs(scaledsize.x * sin(Convert::degreesToRadians(mRotation))) + abs(scaledsize.y * cos(Convert::degreesToRadians(mRotation)));
    int boundingBoxWidth = abs(scaledsize.x * cos(Convert::degreesToRadians(mRotation))) + abs(scaledsize.y * sin(Convert::degreesToRadians(mRotation)));
    return wxSize(boundingBoxWidth, boundingBoxHeight);
}

void VideoKeyFrame::updateAutomatedScaling()
{
    wxSize outputsize{ Properties::get().getVideoSize() };

    switch (mScaling)
    {
    case VideoScalingFitToFill:
    {
        rational64 scalingfactor;
        Convert::sizeInBoundingBox(mInputSize, outputsize, mScalingFactor, true); // The true ensures that the bounding box is filled
        break;
    }
    case VideoScalingFitAll:
    {
        rational64 scalingfactor;
        Convert::sizeInBoundingBox(mInputSize, outputsize, mScalingFactor);
        break;
    }
    case VideoScalingNone:
    {
        mScalingFactor = 1;
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
    ASSERT_DIFFERS(mInputSize, wxSize(0,0));
    wxSize scaledsize = Convert::scale(mInputSize, getScalingFactor());
    wxSize outputsize = Properties::get().getVideoSize();
    wxSize boundingBox = getBoundingBox();

    mRotationPositionOffset = wxPoint((boundingBox.x - scaledsize.x) / 2, (boundingBox.y - scaledsize.y) / 2);

    switch (mAlignment)
    {
    case VideoAlignmentCenter:
    {
        mPosition.x = (outputsize.GetWidth() - scaledsize.GetWidth()) / 2;
        mPosition.y = (outputsize.GetHeight() - scaledsize.GetHeight()) / 2;
        break;
    }
    case VideoAlignmentCenterHorizontal:
    {
        mPosition.x = (outputsize.GetWidth() - scaledsize.GetWidth()) / 2;
        break;
    }
    case VideoAlignmentCenterVertical:
    {
        mPosition.y = (outputsize.GetHeight() - scaledsize.GetHeight()) / 2;
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
        << obj.mPosition;
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
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void VideoKeyFrame::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void VideoKeyFrame::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::VideoKeyFrame)
