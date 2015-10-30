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

#include "VideoClip.h"

#include "Config.h"
#include "Constants.h"
#include "Convert.h"
#include "EmptyFrame.h"
#include "Properties.h"
#include "UtilClone.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "UtilSerializeBoost.h"
#include "UtilSerializeWxwidgets.h"
#include "VideoClipEvent.h"
#include "VideoCompositionParameters.h"
#include "VideoFile.h"
#include "VideoFrameLayer.h"
#include "VideoSkipFrame.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoClip::VideoClip()
    : ClipInterval()
    , mProgress(0)
    , mOpacity(Constants::sOpacityMax)
    , mScaling()
    , mScalingFactor(1)
    , mRotation(0)
    , mRotationPositionOffset(0, 0)
    , mAlignment()
    , mPosition(0, 0)
{
    VAR_DEBUG(*this);
}

VideoClip::VideoClip(const VideoFilePtr& file)
    : ClipInterval(file)
    , mProgress(0)
    , mOpacity(Constants::sOpacityMax)
    , mScaling(Config::ReadEnum<VideoScaling>(Config::sPathVideoDefaultScaling))
    , mScalingFactor(1)
    , mRotation(0)
    , mRotationPositionOffset(0, 0)
    , mAlignment(Config::ReadEnum<VideoAlignment>(Config::sPathVideoDefaultAlignment))
    , mPosition(0, 0)
{
    VAR_DEBUG(*this);
    updateAutomatedScaling();
    updateAutomatedPositioning();
}

VideoClip::VideoClip(const VideoClip& other)
    : ClipInterval(other)
    , mProgress(0)
    , mOpacity(other.mOpacity)
    , mScaling(other.mScaling)
    , mScalingFactor(other.mScalingFactor)
    , mRotation(other.mRotation)
    , mRotationPositionOffset(other.mRotationPositionOffset)
    , mAlignment(other.mAlignment)
    , mPosition(other.mPosition)
{
    VAR_DEBUG(*this)(other);
}

VideoClip* VideoClip::clone() const
{
    return new VideoClip(static_cast<const VideoClip&>(*this));
}

VideoClip::~VideoClip()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

void VideoClip::clean()
{
    VAR_DEBUG(this);
    mProgress = 0;
    ClipInterval::clean();
}

//////////////////////////////////////////////////////////////////////////
// ICLIP
//////////////////////////////////////////////////////////////////////////

std::ostream& VideoClip::dump(std::ostream& os) const
{
    os << *this;
    return os;
}

const char* VideoClip::getType() const
{
    return "Video";
}

//////////////////////////////////////////////////////////////////////////
// IVIDEO
//////////////////////////////////////////////////////////////////////////

VideoFramePtr VideoClip::getNextVideo(const VideoCompositionParameters& parameters)
{
    bool enforceKeyFrame = false;
    if (getNewStartPosition())
    {
        mProgress = *getNewStartPosition(); // Reinitialize mProgress to the last value set in ::moveTo
        invalidateNewStartPosition();
        enforceKeyFrame = true; // Every first frame of a clip is forced to be a key frame
    }

    pts length = getLength();

    VideoFramePtr videoFrame;

    if (mProgress < length)
    {
        VideoFilePtr generator = getDataGenerator<VideoFile>();

        if (!generator->canBeOpened())
        {
            videoFrame = boost::make_shared<VideoFrame>(parameters, boost::make_shared<VideoFrameLayer>(getErrorImage(getDescription())));
        }
        else
        {
            // Scale the clip's size and region of interest to the bounding box
            // Determine scaling for 'fitting' a clip with size 'projectSize' in a bounding box of size 'size'
            wxSize outputsize = Properties::get().getVideoSize();

            boost::rational<int> scaleToBoundingBox(0);
            wxSize requiredOutputSize = Convert::sizeInBoundingBox(outputsize, parameters.getBoundingBox(), scaleToBoundingBox);
            ASSERT_NONZERO(scaleToBoundingBox);
            boost::rational<int> videoscaling = getScalingFactor() * scaleToBoundingBox;
            wxSize inputsize = generator->getSize();
            wxSize requiredVideoSize = Convert::scale(inputsize, videoscaling);

            bool isEmpty = (requiredVideoSize.GetWidth() == 0) || (requiredVideoSize.GetHeight() == 0);

            if (isEmpty)
            {
                videoFrame = boost::make_shared<EmptyFrame>(parameters);
            }
            else
            {
                // IMPORTANT: When getting video frames 'while' playing the timeline, AND resizing the player in parallel, the returned
                //            video frame can have a different size than requested!!! This can happen because the previous frame is returned 'again'.
                //            For this reason, when the videoplayer is resized, playback is stopped.

                VideoCompositionParameters fileparameters(parameters);
                fileparameters.setBoundingBox(requiredVideoSize);
                if (parameters.hasPts())
                {
                    pts requiredPts = Convert::positionToNormalSpeed(getOffset() + parameters.getPts(), getSpeed());
                    fileparameters.setPts(requiredPts);
                }
                VideoFramePtr fileFrame = generator->getNextVideo(fileparameters);
                if (fileFrame)
                {
                    if (parameters.getSkip()) // No need to apply any transformation, since the returned frame is a skip  frame anyway.
                    {
                        videoFrame = fileFrame;
                    }
                    else
                    {
                        ASSERT(!fileFrame->isA<VideoSkipFrame>());
                        ASSERT_EQUALS(fileFrame->getLayers().size(), 1);
                        videoFrame = boost::make_shared<VideoFrame>(parameters, fileFrame->getLayers().front());
                        videoFrame->getLayers().front()->setPosition(Convert::scale(mPosition - mRotationPositionOffset, scaleToBoundingBox));
                        videoFrame->getLayers().front()->setOpacity(mOpacity);
                        videoFrame->getLayers().front()->setRotation(mRotation);
                        videoFrame->setTime(fileFrame->getTime());
                    }
                }
                else
                {
                    // See AudioClip::getNextAudio
                    // The clip has not provided enough video data yet (for the pts length of the clip)
                    // but there is no more video data. This can typically happen by using a avi file
                    // for which the audio data is longer than the video data. Instead of clipping the
                    // extra audio part, empty video is added here (the user can make the clip shorter if
                    // required - thus removing the extra audio, but that's a user decision to be made).
                    LOG_WARNING << *this << ": (" << getDescription() << ") Adding extra video frame to make video length equal to audio length";
                    videoFrame = boost::static_pointer_cast<VideoFrame>(boost::make_shared<EmptyFrame>(parameters));
                }
            }
        }
    }

    if (videoFrame)
    {
        // Note: for some clips, due to framerate resampling, the first and second frames might be the exact same frame. However, only for the first one the keyframe flag is desired
        videoFrame->setForceKeyFrame(enforceKeyFrame);
    }
    mProgress++;
    return videoFrame;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

wxSize VideoClip::getInputSize()
{
    return getDataGenerator<VideoFile>()->getSize();
}

int VideoClip::getOpacity() const
{
    return mOpacity;
}

VideoScaling VideoClip::getScaling() const
{
    return mScaling;
}

boost::rational<int> VideoClip::getScalingFactor() const
{
    return mScalingFactor;
}

boost::rational<int> VideoClip::getRotation() const
{
    return mRotation;
}

VideoAlignment VideoClip::getAlignment() const
{
    return mAlignment;
}

wxPoint VideoClip::getPosition() const
{
    return mPosition;
}

wxPoint VideoClip::getMinPosition()
{
    wxSize boundingBox = getBoundingBox();
    return wxPoint(-boundingBox.x, -boundingBox.y);
}

wxPoint VideoClip::getMaxPosition()
{
    wxSize outputsize = Properties::get().getVideoSize();
    wxSize boundingBox = getBoundingBox();
    int maxX = std::max(boundingBox.x, outputsize.x);
    int maxY = std::max(boundingBox.y, outputsize.y);
    return wxPoint(maxX, maxY) + mRotationPositionOffset;
}

void VideoClip::setOpacity(int opacity)
{
    if (mOpacity != opacity)
    {
        ASSERT_MORE_THAN_EQUALS(opacity, Constants::sOpacityMin);
        ASSERT_LESS_THAN_EQUALS(opacity, Constants::sOpacityMax);
        mOpacity = opacity;
        EventChangeVideoClipOpacity event(mOpacity);
        ProcessEvent(event);
    }
}

void VideoClip::setScaling(const VideoScaling& scaling, const boost::optional<boost::rational< int > >& factor)
{
    VideoScaling oldScaling = mScaling;
    boost::rational<int> oldScalingFactor = mScalingFactor;
    wxPoint oldPosition = mPosition;
    wxPoint oldMinPosition = getMinPosition();
    wxPoint oldMaxPosition = getMaxPosition();

    mScaling = scaling;
    if (factor)
    {
        mScalingFactor = *factor;
    }

    updateAutomatedScaling();
    updateAutomatedPositioning();

    if (mScaling != oldScaling)
    {
        EventChangeVideoClipScaling event(mScaling);
        ProcessEvent(event);
    }
    if (mScalingFactor != oldScalingFactor)
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

void VideoClip::setRotation(const boost::rational< int >& rotation)
{
    boost::rational< int > oldRotation = mRotation;
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

void VideoClip::setAlignment(const VideoAlignment& alignment)
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

void VideoClip::setPosition(const wxPoint& position)
{
    VAR_INFO(position);
    wxPoint oldPosition = mPosition;
    mPosition = position;

    updateAutomatedPositioning();

    if (mPosition != oldPosition)
    {
        EventChangeVideoClipPosition event(mPosition);
        ProcessEvent(event);
    }
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

wxSize VideoClip::getBoundingBox()
{
    wxSize scaledsize = Convert::scale(getInputSize(), getScalingFactor());
    if (mRotation == boost::rational<int>(0))
    {
        return scaledsize;
    }

    int boundingBoxHeight = abs(scaledsize.x * sin(Convert::degreesToRadians(mRotation))) + abs(scaledsize.y * cos(Convert::degreesToRadians(mRotation)));
    int boundingBoxWidth = abs(scaledsize.x * cos(Convert::degreesToRadians(mRotation))) + abs(scaledsize.y * sin(Convert::degreesToRadians(mRotation)));
    return wxSize(boundingBoxWidth, boundingBoxHeight);
}

void VideoClip::updateAutomatedScaling()
{
    wxSize inputsize = getInputSize();
    wxSize outputsize = Properties::get().getVideoSize();

    switch (mScaling)
    {
    case VideoScalingFitToFill:
    {
        boost::rational<int> scalingfactor;
        Convert::sizeInBoundingBox(inputsize, outputsize, mScalingFactor, true); // The true ensures that the bounding box is filled
        break;
    }
    case VideoScalingFitAll:
    {
        boost::rational<int> scalingfactor;
        Convert::sizeInBoundingBox(inputsize, outputsize, mScalingFactor);
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
    if (mScalingFactor > boost::rational<int>(Constants::sScalingMax, model::Constants::sScalingPrecisionFactor))
    {
        mScalingFactor = boost::rational<int>(Constants::sScalingMax, model::Constants::sScalingPrecisionFactor);
    }
    if (mScalingFactor < boost::rational<int>(Constants::sScalingMin, model::Constants::sScalingPrecisionFactor))
    {
        mScalingFactor = boost::rational<int>(Constants::sScalingMin, model::Constants::sScalingPrecisionFactor);
    }
}

void VideoClip::updateAutomatedPositioning()
{
    wxSize inputsize = getInputSize();
    wxSize scaledsize = Convert::scale(inputsize, getScalingFactor());
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

std::ostream& operator<<(std::ostream& os, const VideoClip& obj)
{
    os << static_cast<const ClipInterval&>(obj) << '|'
        << std::setw(4) << obj.mProgress << '|'
        << std::setw(2) << std::hex << obj.mOpacity << std::dec << '|'
        << obj.mScaling << '|'
        << obj.mScalingFactor << '|'
        << obj.mRotation << '|'
        << obj.mAlignment << '|'
        << obj.mPosition;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void VideoClip::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ClipInterval);
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(IVideo);
        ar & BOOST_SERIALIZATION_NVP(mOpacity);
        ar & BOOST_SERIALIZATION_NVP(mScaling);
        ar & BOOST_SERIALIZATION_NVP(mScalingFactor);
        if (version >= 2)
        {
            ar & BOOST_SERIALIZATION_NVP(mRotation);
        }
        if (version >= 3)
        {
            ar & BOOST_SERIALIZATION_NVP(mRotationPositionOffset);
        }
        ar & BOOST_SERIALIZATION_NVP(mAlignment);
        ar & BOOST_SERIALIZATION_NVP(mPosition);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void VideoClip::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void VideoClip::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::VideoClip)
