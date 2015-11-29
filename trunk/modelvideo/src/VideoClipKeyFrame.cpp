// Copyright 2015 Eric Raijmakers.
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

#include "VideoClipKeyFrame.h"

#include "Config.h"
#include "Convert.h"
#include "EmptyFrame.h"
#include "Properties.h"
#include "StatusBar.h"
#include "UtilClone.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "UtilSerializeBoost.h"
#include "UtilSerializeWxwidgets.h"
#include "VideoClipEvent.h"
#include "VideoClipEvent.h" // todo rename to kfe
#include "VideoCompositionParameters.h"
#include "VideoFile.h"
#include "VideoFrameLayer.h"
#include "VideoSkipFrame.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

const rational64 VideoClipKeyFrame::sScalingMin{ 1,100 }; // 0.01
const rational64 VideoClipKeyFrame::sScalingMax{ 100,1 }; // 100

VideoClipKeyFrame::VideoClipKeyFrame()
    : wxEvtHandler{}
    , mIsInterpolated{ false }
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

VideoClipKeyFrame::VideoClipKeyFrame(const wxSize& size)
    : VideoClipKeyFrame{}
{
    VAR_DEBUG(*this);
    mInputSize = size;
    mScaling = Config::ReadEnum<VideoScaling>(Config::sPathVideoDefaultScaling);
    mAlignment = Config::ReadEnum<VideoAlignment>(Config::sPathVideoDefaultAlignment);
    updateAutomatedScaling();
    updateAutomatedPositioning();
}

VideoClipKeyFrame::VideoClipKeyFrame(VideoClipKeyFramePtr before, VideoClipKeyFramePtr after, pts positionBefore, pts position, pts positionAfter)
    : VideoClipKeyFrame{}
{
    ASSERT_EQUALS(before->getSize(), after->getSize());
    ASSERT_LESS_THAN(positionBefore, position);
    ASSERT_LESS_THAN(position, positionAfter);
    rational64 factor{ position - positionBefore, positionAfter - positionBefore };

    mIsInterpolated = true;
    mInputSize = before->getSize();
    mOpacity = before->getOpacity() + boost::rational_cast<int>(factor * (rational64(after->getOpacity() - before->getOpacity())));
    mScaling = model::VideoScalingCustom;
    mScalingFactor = before->getScalingFactor() + (factor * (rational64(after->getScaling() - before->getScaling())));
    mRotation = before->getRotation() + (factor * (rational64(after->getRotation() - before->getRotation())));
    mRotationPositionOffset.x = before->getRotationPositionOffset().x + boost::rational_cast<int>(factor * (rational64(after->getRotationPositionOffset().x - before->getRotationPositionOffset().x)));
    mRotationPositionOffset.y = before->getRotationPositionOffset().y + boost::rational_cast<int>(factor * (rational64(after->getRotationPositionOffset().y - before->getRotationPositionOffset().y)));
    mAlignment = model::VideoAlignmentCustom;
    mPosition.x = before->getPosition().x + boost::rational_cast<int>(factor * (rational64(after->getPosition().x - before->getPosition().x)));
    mPosition.y = before->getPosition().y + boost::rational_cast<int>(factor * (rational64(after->getPosition().y - before->getPosition().y)));
}

VideoClipKeyFrame::VideoClipKeyFrame(const VideoClipKeyFrame& other)
    : wxEvtHandler{}
    , mIsInterpolated{ other.mIsInterpolated }
    , mInputSize{ other.mInputSize }
    , mOpacity{ other.mOpacity }
    , mScaling{ other.mScaling }
    , mScalingFactor{ other.mScalingFactor }
    , mRotation{ other.mRotation }
    , mRotationPositionOffset{ other.mRotationPositionOffset }
    , mAlignment{ other.mAlignment }
    , mPosition{ other.mPosition }
{
    VAR_DEBUG(*this)(other);
}

VideoClipKeyFrame* VideoClipKeyFrame::clone() const
{
    return new VideoClipKeyFrame(static_cast<const VideoClipKeyFrame&>(*this));
}

VideoClipKeyFrame::~VideoClipKeyFrame()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

int VideoClipKeyFrame::getOpacity() const
{
    return mOpacity;
}

VideoScaling VideoClipKeyFrame::getScaling() const
{
    return mScaling;
}

rational64 VideoClipKeyFrame::getScalingFactor() const
{
    return mScalingFactor;
}

rational64 VideoClipKeyFrame::getRotation() const
{
    return mRotation;
}

wxPoint VideoClipKeyFrame::getRotationPositionOffset() const
{
    return mRotationPositionOffset;
}

VideoAlignment VideoClipKeyFrame::getAlignment() const
{
    return mAlignment;
}

wxPoint VideoClipKeyFrame::getPosition() const
{
    return mPosition;
}

wxPoint VideoClipKeyFrame::getMinPosition()
{
    wxSize boundingBox = getBoundingBox();
    return wxPoint(-boundingBox.x, -boundingBox.y);
}

wxPoint VideoClipKeyFrame::getMaxPosition()
{
    wxSize outputsize = Properties::get().getVideoSize();
    wxSize boundingBox = getBoundingBox();
    int maxX = std::max(boundingBox.x, outputsize.x);
    int maxY = std::max(boundingBox.y, outputsize.y);
    return wxPoint(maxX, maxY) + mRotationPositionOffset;
}

void VideoClipKeyFrame::setOpacity(int opacity)
{
    if (mOpacity != opacity)
    {
        ASSERT_MORE_THAN_EQUALS(opacity, sOpacityMin);
        ASSERT_LESS_THAN_EQUALS(opacity, sOpacityMax);
        mOpacity = opacity;
        EventChangeVideoClipOpacity event(mOpacity);
        ProcessEvent(event);
    }
}

void VideoClipKeyFrame::setScaling(const VideoScaling& scaling, const boost::optional<rational64 >& factor)
{
    VideoScaling oldScaling = mScaling;
    rational64 oldScalingFactor = mScalingFactor;
    wxPoint oldPosition = mPosition;
    wxPoint oldMinPosition = getMinPosition();
    wxPoint oldMaxPosition = getMaxPosition();

    mScaling = scaling;
    if (factor)
    {
        unsigned int w{ boost::rational_cast<unsigned int>(mInputSize.GetWidth() * *factor) };
        unsigned int h{ boost::rational_cast<unsigned int>(mInputSize.GetHeight() * *factor) };
        if (w <= 0)
        {
            gui::StatusBar::get().timedInfoText(_("Width becomes 0."));
        }
        else if (h <= 0)
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

    if (mScaling != oldScaling || // Update of the change
        mScaling != scaling)      // Inform that the change requested was not acknowledged
    {
        EventChangeVideoClipScaling event(mScaling);
        ProcessEvent(event);
    }
    if (mScalingFactor != oldScalingFactor ||  // Update of the change
        (factor && mScalingFactor != *factor)) // Inform that the change requested was not acknowledged
    {
        EventChangeVideoClipScalingFactor event(mScalingFactor);
        ProcessEvent(event);
    }
    if (getMinPosition() != oldMinPosition)
    {
        EventChangeVideoClipMinPosition event(getMinPosition());
        ProcessEvent(event);
    }
    if (getMaxPosition() != oldMaxPosition)
    {
        EventChangeVideoClipMaxPosition event(getMaxPosition());
        ProcessEvent(event);
    }
    if (mPosition != oldPosition)
    {
        EventChangeVideoClipPosition event(mPosition);
        ProcessEvent(event);
    }
}

void VideoClipKeyFrame::setRotation(const rational64& rotation)
{
    rational64 oldRotation = mRotation;
    wxPoint oldPosition = mPosition;
    wxPoint oldMinPosition = getMinPosition();
    wxPoint oldMaxPosition = getMaxPosition();

    mRotation = rotation;

    updateAutomatedPositioning();
    if (mRotation != oldRotation)
    {
        EventChangeVideoClipRotation event(mRotation);
        ProcessEvent(event);
    }
    if (getMinPosition() != oldMinPosition)
    {
        EventChangeVideoClipMinPosition event(getMinPosition());
        ProcessEvent(event);
    }
    if (getMaxPosition() != oldMaxPosition)
    {
        EventChangeVideoClipMaxPosition event(getMaxPosition());
        ProcessEvent(event);
    }
    if (mPosition != oldPosition)
    {
        EventChangeVideoClipPosition event(mPosition);
        ProcessEvent(event);
    }
}

void VideoClipKeyFrame::setRotationPositionOffset(wxPoint position)
{
    mRotationPositionOffset = position;
}

void VideoClipKeyFrame::setAlignment(const VideoAlignment& alignment)
{
    VideoAlignment oldAlignment = mAlignment;
    wxPoint oldPosition = mPosition;

    mAlignment = alignment;

    updateAutomatedPositioning();

    if (mAlignment != oldAlignment)
    {
        EventChangeVideoClipAlignment event(mAlignment);
        ProcessEvent(event);
    }
    if (mPosition != oldPosition)
    {
        EventChangeVideoClipPosition event(mPosition);
        ProcessEvent(event);
    }
}

void VideoClipKeyFrame::setPosition(const wxPoint& position)
{
    VAR_INFO(position);
    wxPoint oldPosition = mPosition;
    mPosition = position;

    updateAutomatedPositioning();

    if (mPosition != oldPosition ||  // Update of the change
        mPosition != position)       // Inform that the change requested was not acknowledged
    {
        EventChangeVideoClipPosition event(mPosition);
        ProcessEvent(event);
    }
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

wxSize VideoClipKeyFrame::getBoundingBox()
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

void VideoClipKeyFrame::updateAutomatedScaling()
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
    if (mScalingFactor > VideoClipKeyFrame::sScalingMax)
    {
        mScalingFactor = VideoClipKeyFrame::sScalingMax;
    }
    if (mScalingFactor < VideoClipKeyFrame::sScalingMin)
    {
        mScalingFactor = VideoClipKeyFrame::sScalingMin;
    }
}

void VideoClipKeyFrame::updateAutomatedPositioning()
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

std::ostream& operator<<(std::ostream& os, const VideoClipKeyFrame& obj)
{
    os << std::setw(4) << obj.mInputSize << '|'
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
void VideoClipKeyFrame::serialize(Archive & ar, const unsigned int version)
{
    try
    {
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
template void VideoClipKeyFrame::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void VideoClipKeyFrame::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::VideoClipKeyFrame)
