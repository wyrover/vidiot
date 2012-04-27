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
#include "UtilSerializeWxwidgets.h"
#include "VideoClipEvent.h"

namespace model {
//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoClip::VideoClip()
    : Clip()
    , mProgress(0)
    , mScaling()
    , mScalingFactor(1)
    , mAlignment()
    , mRegionOfInterest(0,0,0,0)
{
    VAR_DEBUG(this);
}

VideoClip::VideoClip(IControlPtr file)
    : Clip(file)
    , mProgress(0)
    , mScaling(gui::Config::ReadEnum<VideoScaling>(gui::Config::sPathDefaultVideoScaling))
    , mScalingFactor(1)
    , mAlignment(gui::Config::ReadEnum<VideoAlignment>(gui::Config::sPathDefaultVideoAlignment))
    , mRegionOfInterest()
{
    VAR_DEBUG(this);
    setScaling(mScaling);
    determineTransform();
}

VideoClip::VideoClip(const VideoClip& other)
    : Clip(other)
    , mProgress(0)
    , mScaling(other.mScaling)
    , mScalingFactor(other.mScalingFactor)
    , mAlignment(other.mAlignment)
    , mRegionOfInterest(other.mRegionOfInterest)
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
        // Scale the clip's size and region of interest to the bounding box
        // Determine scaling for 'fitting' a clip with size 'projectSize' in a bounding box of size 'size'
        double scaleToBoundingBox(0);
        wxSize requiredOutputSize = Convert::sizeInBoundingBox(mRegionOfInterest.GetSize(), size, scaleToBoundingBox);
        ASSERT_NONZERO(scaleToBoundingBox);
        VideoFilePtr generator = getDataGenerator<VideoFile>();
        wxSize requiredVideoSize = Convert::scale(generator->getSize(), mScalingFactor * scaleToBoundingBox);
        wxRect regionOfInterest(Convert::scale(mRegionOfInterest, scaleToBoundingBox));
        VAR_INFO(requiredVideoSize)(regionOfInterest)(scaleToBoundingBox);
        ASSERT_MORE_THAN_EQUALS_ZERO(regionOfInterest.GetX());
        ASSERT_MORE_THAN_EQUALS_ZERO(regionOfInterest.GetY());
        ASSERT_LESS_THAN_EQUALS(regionOfInterest.GetWidth(), size.x);
        ASSERT_LESS_THAN_EQUALS(regionOfInterest.GetHeight(), size.y); // todo assert goes off when resizing (reducing size) with the left edge

        //VideoFilePtr generator = getDataGenerator<VideoFile>();
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

wxSize VideoClip::getInputSize()
{
    return getDataGenerator<VideoFile>()->getSize();
}

VideoScaling VideoClip::getScaling() const
{
    return mScaling;
}

double VideoClip::getScalingFactor() const
{
    return mScalingFactor;
}

wxSize VideoClip::getSize() const
{
    // Without scaling: use default project size
    // Todo add scaling (make smaller) for compositing
    return Properties::get()->getVideoSize();
}

VideoAlignment VideoClip::getAlignment() const
{
    return mAlignment;
}

wxRect VideoClip::getRegionOfInterest() const
{
    return mRegionOfInterest;
}

void VideoClip::determineTransform()
{
    wxSize inputVideoSize = getInputSize();
    wxSize projectSize = Properties::get()->getVideoSize();

    // Determine scaling the clip to the project size
    //mScalingFactor = determineScalingFactor(inputVideoSize,projectSize,mScaling);
    // todo rename the rest of this method into set alignment

    wxSize scaledsize = Convert::scale(inputVideoSize,mScalingFactor);
    mRegionOfInterest = determineRegionOfInterest(scaledsize,projectSize,mAlignment);

    VAR_DEBUG(mScaling)(mScalingFactor)(mAlignment)(mRegionOfInterest);
}

void VideoClip::setScaling(VideoScaling scaling, boost::optional<double> factor)
{
    wxSize inputsize = getInputSize();
    wxSize outputsize = Properties::get()->getVideoSize();

    double result = 0;
    wxSize scaledsize(0,0);
    switch (scaling)
    {
    case VideoScalingFitToFill:
        scaledsize = Convert::fillBoundingBoxWithMinimalLoss(inputsize, outputsize, result); // todo don't use project default but give each clip it's own setting!
        break;
    case VideoScalingFitAll:
        scaledsize = Convert::sizeInBoundingBox(inputsize, outputsize, result);
        break;
    case VideoScalingNone:
        result = 1.0;
        break;
    case VideoScalingCustom:
        if (factor)
        {
            result = *factor;
        }
        else
        {
            result = mScaling; // Use current scaling factor
        }
        break;
    default:
        NIY;
        break;
    }
    ASSERT_NONZERO(result);
    mScaling = scaling;
    mScalingFactor = result;
    ProcessEvent(EventChangeVideoClipScaling(mScaling));
    ProcessEvent(EventChangeVideoClipScalingFactor(mScalingFactor));
}

wxRect VideoClip::determineRegionOfInterest(wxSize inputsize, wxSize outputsize, VideoAlignment alignment)
{
    wxRect roi(wxPoint(0,0),outputsize);
    switch (alignment)
    {
    case VideoAlignmentCenter:
        {
            wxSize diff = inputsize - outputsize;
            diff /= 2;
            roi.SetPosition(wxPoint(0,0) + diff);
            break;
        }
    case VideoAlignmentCustom:
    default:
        NIY;
        break;
    }
    return roi;
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
    ar & mScaling;
    ar & mScalingFactor;
    ar & mAlignment;
    ar & mRegionOfInterest;
}
template void VideoClip::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void VideoClip::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);
} //namespace