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

#include "VideoClip.h"

#include "Config.h"
#include "Constants.h"
#include "Convert.h"
#include "EmptyFrame.h"
#include "Properties.h"
#include "UtilCloneable.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "UtilSerializeBoost.h"
#include "UtilSerializeWxwidgets.h"
#include "VideoClipEvent.h"
#include "VideoCompositionParameters.h"
#include "VideoFile.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoClip::VideoClip()
    : ClipInterval()
    , mProgress(0)
    , mOpacity(Constants::sMaxOpacity)
    , mScaling()
    , mScalingFactor(1)
    , mAlignment()
    , mPosition(0,0)
{
    VAR_DEBUG(*this);
}

VideoClip::VideoClip(VideoFilePtr file)
    : ClipInterval(file)
    , mProgress(0)
    , mOpacity(Constants::sMaxOpacity)
    , mScaling(Config::ReadEnum<VideoScaling>(Config::sPathDefaultVideoScaling))
    , mScalingFactor(1)
    , mAlignment(Config::ReadEnum<VideoAlignment>(Config::sPathDefaultVideoAlignment))
    , mPosition(0,0)
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

char* VideoClip::getType() const
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

    int length = getLength();

    VideoFramePtr videoFrame;

    if (mProgress < length)
    {
        // Scale the clip's size and region of interest to the bounding box
        // Determine scaling for 'fitting' a clip with size 'projectSize' in a bounding box of size 'size'
        wxSize outputsize = Properties::get().getVideoSize();

        boost::rational<int> scaleToBoundingBox(0);
        wxSize requiredOutputSize = Convert::sizeInBoundingBox(outputsize, parameters.getBoundingBox(), scaleToBoundingBox);
        ASSERT_NONZERO(scaleToBoundingBox);
        VideoFilePtr generator = getDataGenerator<VideoFile>();
        boost::rational<int> videoscaling = getScalingFactor() * scaleToBoundingBox;
        wxSize inputsize = generator->getSize();
        wxSize requiredVideoSize = Convert::scale(inputsize, videoscaling);

        bool isEmpty = (requiredVideoSize.GetWidth() == 0) || (requiredVideoSize.GetHeight() == 0);

        if (isEmpty)
        {
            VAR_WARNING(isEmpty)(requiredVideoSize);
            videoFrame = boost::make_shared<EmptyFrame>(requiredVideoSize,mProgress);
        }
        else
        {
            // IMPORTANT: When getting video frames 'while' playing the timeline, AND resizing the player in parallel, the returned
            //            video frame can have a different size than requested!!! This can happen because the previous frame is returned 'again'.
            //            For this reason, when the videoplayer is resized, playback is stopped.
            videoFrame = generator->getNextVideo(VideoCompositionParameters(parameters).setBoundingBox(requiredVideoSize));
            if (videoFrame)
            {
                wxSize inputsize = getInputSize();
                wxSize outputsize = Properties::get().getVideoSize();
                wxSize scaledsize = Convert::scale(inputsize,getScalingFactor());
                wxRect roi(wxPoint(0,0),scaledsize);

                auto determineroi = [](pixel area, pixel data, pixel& data_pos, pixel& roi_pos, pixel& roi_size)
                {
                    roi_pos = 0;
                    roi_size = data;

                    if (data_pos < 0)
                    {
                        // clip to the left/top
                        ASSERT_MORE_THAN_EQUALS(data_pos, -1 * data);
                        roi_pos += -1 * data_pos;
                        roi_size -= -1 * data_pos;
                        data_pos = 0;
                    }
                    else
                    {
                        // clip to the right/bottom
                        ASSERT_LESS_THAN_EQUALS(data_pos,area);
                        if (data_pos + data > area)
                        {
                            roi_size = area - data_pos;
                        }
                    }
                    roi_size = std::min(roi_size,area);
                    ASSERT_MORE_THAN_EQUALS_ZERO(roi_pos);
                    ASSERT_MORE_THAN_EQUALS_ZERO(roi_size);

                };

                wxPoint position(mPosition);
                determineroi(outputsize.x,scaledsize.x,position.x,roi.x,roi.width);
                determineroi(outputsize.y,scaledsize.y,position.y,roi.y,roi.height);
                videoFrame->setRegionOfInterest(Convert::scale(roi, scaleToBoundingBox));
                videoFrame->setPosition(Convert::scale(position, scaleToBoundingBox));
                videoFrame->setOpacity(mOpacity);
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

                videoFrame = boost::static_pointer_cast<VideoFrame>(boost::make_shared<EmptyFrame>(parameters.getBoundingBox(), mProgress));
            }
        }

        mProgress++;
    }

    if (videoFrame)
    {
        // Note: for some clips, due to framerate resampling, the first and second frames might be the exact same frame. However, only for the first one the keyframe flag is desired
        videoFrame->setForceKeyFrame(enforceKeyFrame);
    }
    VAR_VIDEO(videoFrame);
    setGenerationProgress(mProgress);
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

void VideoClip::setOpacity(int opacity)
{
    if (mOpacity != opacity)
    {
        ASSERT_MORE_THAN_EQUALS(opacity,Constants::sMinOpacity);
        ASSERT_LESS_THAN_EQUALS(opacity,Constants::sMaxOpacity);
        mOpacity = opacity;
        ProcessEvent(EventChangeVideoClipOpacity(mOpacity));
    }
}

VideoScaling VideoClip::getScaling() const
{
    return mScaling;
}

boost::rational<int> VideoClip::getScalingFactor() const
{
    return mScalingFactor;
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
    wxSize inputsize = getInputSize();
    wxSize scaledsize = Convert::scale(inputsize,getScalingFactor());
    return wxPoint(-1 * scaledsize.x, -1 * scaledsize.y);
}

wxPoint VideoClip::getMaxPosition()
{
    wxSize outputsize = Properties::get().getVideoSize();
    return wxPoint(outputsize.x,outputsize.y);
}

void VideoClip::setScaling(VideoScaling scaling, boost::optional<boost::rational< int > > factor)
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
        ProcessEvent(EventChangeVideoClipScaling(mScaling));
    }
    if (mScalingFactor != oldScalingFactor)
    {
        ProcessEvent(EventChangeVideoClipScalingFactor(mScalingFactor));
    }
    if (getMinPosition() != oldMinPosition)
    {
        ProcessEvent(EventChangeVideoClipMinPosition(getMinPosition()));
    }
    if (getMaxPosition() != oldMaxPosition)
    {
        ProcessEvent(EventChangeVideoClipMaxPosition(getMaxPosition()));
    }
    if (mPosition != oldPosition)
    {
        ProcessEvent(EventChangeVideoClipPosition(mPosition));
    }
}

void VideoClip::setAlignment(VideoAlignment alignment)
{
    VideoAlignment oldAlignment = mAlignment;
    wxPoint oldPosition = mPosition;

    mAlignment = alignment;

    updateAutomatedPositioning();

    if (mAlignment != oldAlignment)
    {
        ProcessEvent(EventChangeVideoClipAlignment(mAlignment));
    }
    if (mPosition != oldPosition)
    {
        ProcessEvent(EventChangeVideoClipPosition(mPosition));
    }
}

void VideoClip::setPosition(wxPoint position)
{
    VAR_INFO(position);
    wxPoint oldPosition = mPosition;
    mPosition = position;

    updateAutomatedPositioning();

    if (mPosition != oldPosition)
    {
        ProcessEvent(EventChangeVideoClipPosition(mPosition));
    }
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

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

    ASSERT_LESS_THAN_EQUALS(mScalingFactor,boost::rational<int>(Constants::sMaxScaling,model::Constants::scalingPrecisionFactor));
    ASSERT_MORE_THAN_EQUALS(mScalingFactor,boost::rational<int>(Constants::sMinScaling,model::Constants::scalingPrecisionFactor));
}

void VideoClip::updateAutomatedPositioning()
{
    wxSize inputsize = getInputSize();
    wxSize scaledsize = Convert::scale(inputsize,getScalingFactor());
    wxSize outputsize = Properties::get().getVideoSize();
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

std::ostream& operator<<( std::ostream& os, const VideoClip& obj )
{
    os << static_cast<const ClipInterval&>(obj) << '|' << std::setw(4) << obj.mProgress << '|' << std::setw(2) << std::hex << obj.mOpacity <<  std::dec << '|' << obj.mScaling << '|' << obj.mScalingFactor << '|' << obj.mAlignment << '|' << obj.mPosition;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void VideoClip::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<ClipInterval>(*this);
    ar & boost::serialization::base_object<IVideo>(*this);
    ar & mOpacity;
    ar & mScaling;
    ar & mScalingFactor;
    ar & mAlignment;
    ar & mPosition;
}
template void VideoClip::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void VideoClip::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace