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
#include "Convert.h"
#include "EmptyFrame.h"
#include "Properties.h"
#include "StatusBar.h"
#include "UtilClone.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "UtilSerializeBoost.h"
#include "UtilSerializeWxwidgets.h"
#include "VideoCompositionParameters.h"
#include "VideoFile.h"
#include "VideoFrameLayer.h"
#include "VideoKeyFrame.h"
#include "VideoSkipFrame.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoClip::VideoClip()
    : ClipInterval()
    , mProgress(0)
{
    VAR_DEBUG(*this);
}

VideoClip::VideoClip(const VideoFilePtr& file)
    : ClipInterval(file)
    , mProgress(0)
{
    VAR_DEBUG(*this);
    setDefaultKeyFrame(boost::make_shared<VideoKeyFrame>(file->getSize()));
}

VideoClip::VideoClip(const VideoClip& other)
    : ClipInterval(other)
    , mProgress(0)
{
    VAR_DEBUG(other)(*this);
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
            VideoKeyFramePtr keyFrame{ boost::dynamic_pointer_cast<VideoKeyFrame>(getFrameAt(mProgress + getOffset() - getPerceivedOffset())) };

            // Scale the clip's size and region of interest to the bounding box
            // Determine scaling for 'fitting' a clip with size 'projectSize' in a bounding box of size 'size'
            wxSize outputsize = Properties::get().getVideoSize();

            rational64 scaleToBoundingBox(0);
            wxSize requiredOutputSize = Convert::sizeInBoundingBox(outputsize, parameters.getBoundingBox(), scaleToBoundingBox);
            ASSERT_NONZERO(scaleToBoundingBox);
            rational64 videoscaling = keyFrame->getScalingFactor() * scaleToBoundingBox;
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
                        videoFrame->getLayers().front()->setPosition(Convert::scale(keyFrame->getPosition() - keyFrame->getRotationPositionOffset(), scaleToBoundingBox));
                        videoFrame->getLayers().front()->setOpacity(keyFrame->getOpacity());
                        videoFrame->getLayers().front()->setRotation(keyFrame->getRotation());
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

//////////////////////////////////////////////////////////////////////////
// KEY FRAMES
//////////////////////////////////////////////////////////////////////////

KeyFramePtr VideoClip::interpolate(KeyFramePtr before, KeyFramePtr after, pts positionBefore, pts position, pts positionAfter) const
{
    return boost::make_shared<VideoKeyFrame>(boost::dynamic_pointer_cast<VideoKeyFrame>(before), boost::dynamic_pointer_cast<VideoKeyFrame>(after), positionBefore, position, positionAfter);
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const VideoClip& obj)
{
    os << static_cast<const ClipInterval&>(obj) << '|'
        << std::setw(4) << obj.mProgress;
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
        if (version < 4)
        {
            int mOpacity;
            VideoScaling mScaling;
            rational32 mScalingFactor{ 1 };
            rational32 mRotation{ 0 };
            wxPoint mRotationPositionOffset{ 0,0 };
            VideoAlignment mAlignment;
            wxPoint mPosition;
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
            VideoKeyFramePtr keyFrame{ boost::make_shared<VideoKeyFrame>(getInputSize()) };
            keyFrame->setOpacity(mOpacity);
            rational64 scalingFactor64{ mScalingFactor.numerator(), mScalingFactor.denominator() };
            keyFrame->setScaling(mScaling, scalingFactor64);
            keyFrame->setRotation(rational64{ mRotation.numerator(), mRotation.denominator() });
            keyFrame->setRotationPositionOffset(mRotationPositionOffset);
            keyFrame->setAlignment(mAlignment);
            keyFrame->setPosition(mPosition);
            setDefaultKeyFrame(keyFrame);
        }
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void VideoClip::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void VideoClip::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::VideoClip)
