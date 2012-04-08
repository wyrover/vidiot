#include "VideoClip.h"

#include <boost/make_shared.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "Config.h"
#include "Constants.h"
#include "Convert.h"
#include "EmptyFrame.h"
#include "Properties.h"
#include "UtilLog.h"
#include "VideoFile.h"
#include "UtilLogWxwidgets.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoClip::VideoClip()
    : Clip()
    , mProgress(0)
    , mScaling(gui::Config::ReadEnum<VideoScaling>(gui::Config::sPathDefaultVideoScaling))
    , mAlignment(gui::Config::ReadEnum<VideoAlignment>(gui::Config::sPathDefaultVideoAlignment))
{
    VAR_DEBUG(this);
}

VideoClip::VideoClip(IControlPtr file)
    : Clip(file)
    , mProgress(0)
    , mScaling(gui::Config::ReadEnum<VideoScaling>(gui::Config::sPathDefaultVideoScaling))
    , mAlignment(gui::Config::ReadEnum<VideoAlignment>(gui::Config::sPathDefaultVideoAlignment))
{
    VAR_DEBUG(this);
}

VideoClip::VideoClip(const VideoClip& other)
    : Clip(other)
    , mProgress(0)
    , mScaling(other.mScaling)
    , mAlignment(other.mAlignment)
{
    VAR_DEBUG(*this);
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
    Clip::clean();
}

//////////////////////////////////////////////////////////////////////////
// IVIDEO
//////////////////////////////////////////////////////////////////////////

VideoFramePtr VideoClip::getNextVideo(wxSize size, bool alpha)
{
    VAR_INFO(size);
    if (getLastSetPosition())
    {
        mProgress = *getLastSetPosition(); // Reinitialize mProgress to the last value set in ::moveTo
        invalidateLastSetPosition();
    }

    unsigned int length = getLength();

    VideoFramePtr videoFrame;

    if (mProgress < length)
    {
        VideoFilePtr generator = getDataGenerator<VideoFile>();

        wxSize inputVideoSize = generator->getSize();
        wxSize projectSize = Properties::get()->getVideoSize();

        // Determine scaling the clip to the project size
        double scaleInputSizeToProjectSize(0);
        wxSize scaledToProjectSize(0,0);
        switch (mScaling)
        {
        case VideoScalingFitToFill:
            scaledToProjectSize = Convert::fillBoundingBoxWithMinimalLoss(inputVideoSize, projectSize, scaleInputSizeToProjectSize); // todo don't use project default but give each clip it's own setting!
            break;
        case VideoScalingFitAll:
            scaledToProjectSize = Convert::sizeInBoundingBox(inputVideoSize, projectSize, scaleInputSizeToProjectSize);
            break;
        case VideoScalingNone:
        case VideoScalingCustom:
        default:
            NIY;
            break;
        }
        ASSERT_NONZERO(scaleInputSizeToProjectSize);

        // Determine scaling the project size to the bounding box
        double scaleProjectSizeToBoundingBox(0);
        wxSize requiredOutputSize = Convert::sizeInBoundingBox(projectSize, size, scaleProjectSizeToBoundingBox); // Determine scaling for 'fitting' a clip with size 'projectSize' in a bounding box of size 'size'
        ASSERT_NONZERO(scaleProjectSizeToBoundingBox);

        // Determine region of interest for the project size (this includes clipping)
        wxPoint regionOfInterestPosition(0,0);
        wxSize regionOfInterestSize(projectSize);
        switch (mAlignment)
        {
        case VideoAlignmentCenter:
            {
                wxSize diff = scaledToProjectSize - projectSize;
                diff /= 2;
                regionOfInterestPosition = wxPoint(0,0) + diff;
                break;
            }
        case VideoAlignmentCustom:
        default:
            NIY;
            break;
        }

        // Scale to the given bounding box
        wxSize requiredVideoSize = Convert::scale(scaledToProjectSize, scaleProjectSizeToBoundingBox);
        regionOfInterestPosition = Convert::scale(regionOfInterestPosition, scaleProjectSizeToBoundingBox);
        regionOfInterestSize     = Convert::scale(regionOfInterestSize, scaleProjectSizeToBoundingBox);

        wxRect regionOfInterest(regionOfInterestPosition,regionOfInterestSize);
        VAR_INFO(inputVideoSize)(size)(projectSize)(scaledToProjectSize)(requiredVideoSize)(regionOfInterest)(scaleInputSizeToProjectSize)(scaleProjectSizeToBoundingBox);
        ASSERT_MORE_THAN_EQUALS_ZERO(regionOfInterestPosition.x);
        ASSERT_MORE_THAN_EQUALS_ZERO(regionOfInterestPosition.y);
        ASSERT_LESS_THAN_EQUALS(regionOfInterestSize.x, size.x);
        ASSERT_LESS_THAN_EQUALS(regionOfInterestSize.y, size.y);

        videoFrame = generator->getNextVideo(requiredVideoSize, alpha);
        if (videoFrame)
        {
            ASSERT_MORE_THAN_ZERO(videoFrame->getRepeat());
            if (mProgress + videoFrame->getRepeat() > length)
            {
                videoFrame->setRepeat(length - mProgress);
                mProgress = length;
            }
            else
            {
                mProgress += videoFrame->getRepeat();
            }

            videoFrame->setRegionOfInterest(regionOfInterest);
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

            videoFrame = boost::static_pointer_cast<VideoFrame>(boost::make_shared<EmptyFrame>(alpha ? videoRGBA : videoRGB, size, mProgress));

            mProgress += 1;
        }
    }

    VAR_VIDEO(videoFrame);
    setGenerationProgress(mProgress);
    return videoFrame;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

wxSize VideoClip::getSize()
{
    // Without scaling: use default project size
    // Todo add scaling (make smaller) for compositing
    return Properties::get()->getVideoSize();
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const VideoClip& obj )
{
    os << static_cast<const Clip&>(obj) << '|' << obj.mProgress;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void VideoClip::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<Clip>(*this);
    ar & boost::serialization::base_object<IVideo>(*this);
}
template void VideoClip::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void VideoClip::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace