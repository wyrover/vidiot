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
    , mAlignmentOffset(0,0)
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
    , mAlignmentOffset(0,0)
    , mRegionOfInterest()
{
    VAR_DEBUG(this);
    setScaling(mScaling);
    //determineTransform(); todo
}

VideoClip::VideoClip(const VideoClip& other)
    : Clip(other)
    , mProgress(0)
    , mScaling(other.mScaling)
    , mScalingFactor(other.mScalingFactor)
    , mAlignment(other.mAlignment)
    , mAlignmentOffset(other.mAlignmentOffset)
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
            wxSize outputsize = Properties::get()->getVideoSize();

        double scaleToBoundingBox(0);
//        wxSize requiredOutputSize = Convert::sizeInBoundingBox(mRegionOfInterest.GetSize(), size, scaleToBoundingBox);
                wxSize requiredOutputSize = Convert::sizeInBoundingBox(outputsize, size, scaleToBoundingBox);
        ASSERT_NONZERO(scaleToBoundingBox);
        VideoFilePtr generator = getDataGenerator<VideoFile>();
        double videoscaling = mScalingFactor * scaleToBoundingBox;
        wxSize inputsize = generator->getSize();
        wxSize requiredVideoSize = Convert::scale(inputsize, videoscaling);
        wxRect regionOfInterest(Convert::scale(mRegionOfInterest, scaleToBoundingBox));
        wxPoint offset(Convert::scale(mAlignmentOffset, scaleToBoundingBox));
        VAR_INFO(inputsize)(requiredVideoSize)(regionOfInterest)(mScalingFactor)(scaleToBoundingBox)(videoscaling);
        ASSERT_MORE_THAN_EQUALS_ZERO(regionOfInterest.x);
        ASSERT_MORE_THAN_EQUALS_ZERO(regionOfInterest.y);
        ASSERT_LESS_THAN_EQUALS(regionOfInterest.x + regionOfInterest.width,  requiredVideoSize.x);
        ASSERT_LESS_THAN_EQUALS(regionOfInterest.y + regionOfInterest.height, requiredVideoSize.y);

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
            videoFrame->setPosition(offset);
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
    NIY;
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

    mScaling = scaling;
    switch (mScaling)
    {
    case VideoScalingFitToFill:
        Convert::fillBoundingBoxWithMinimalLoss(inputsize, outputsize, mScalingFactor); // todo don't use project default but give each clip it's own setting!
        break;
    case VideoScalingFitAll:
        Convert::sizeInBoundingBox(inputsize, outputsize, mScalingFactor);
        break;
    case VideoScalingNone:
        mScalingFactor = 1.0;
        break;
    case VideoScalingCustom:
        if (factor)
        {
            mScalingFactor = *factor;
        }
        // else: Use current scaling factor
        break;
    default:
        NIY;
        break;
    }
    ASSERT_NONZERO(mScalingFactor);

    ////////////////////////////////////////////////////////////////////////////////////
    // Determine region of interest and offset

    // todo always update all these transform vars in one action???, yes, make one method with all optionals for 'to be changeds' then make dedicated change methods which trigger this one method
    wxSize scaledsize = Convert::scale(inputsize,mScalingFactor);
    //mRegionOfInterest = determineRegionOfInterest(scaledsize,outputsize,mAlignment);

    mRegionOfInterest.SetPosition(wxPoint(0,0));
    mRegionOfInterest.SetSize(scaledsize);

    switch (mAlignment)
    {
    case VideoAlignmentCenter:
        {
            mAlignmentOffset = wxPoint(0,0);
            pixel ydiff = scaledsize.y - outputsize.y;
            if (scaledsize.GetWidth() >= outputsize.GetWidth()) // Input is larger than required size. Reduce input size by setting region of interest.
            {
                mRegionOfInterest.SetX((scaledsize.GetWidth() - outputsize.GetWidth()) / 2);
                mRegionOfInterest.SetWidth(outputsize.GetWidth());
            }
            else // Input is smaller than required size. Entire input is used.
            {
                mRegionOfInterest.SetX(0);
                mRegionOfInterest.SetWidth(scaledsize.GetWidth());
                mAlignmentOffset.x = (outputsize.GetWidth() - scaledsize.GetWidth()) / 2; // Set offset to ensure that the remaining smaller clip is centered.
            }
            if (scaledsize.GetHeight() >= outputsize.GetHeight()) // Input is larger than required size. Reduce input size by setting region of interest.
            {
                mRegionOfInterest.SetY((scaledsize.GetHeight() - outputsize.GetHeight()) / 2);
                mRegionOfInterest.SetHeight(outputsize.GetHeight());
            }
            else // Input is smaller than required size. Entire input is used.
            {
                mRegionOfInterest.SetY(0);
                mRegionOfInterest.SetHeight(scaledsize.GetHeight());
                mAlignmentOffset.y = (outputsize.GetHeight() - scaledsize.GetHeight()) / 2; // Set offset to ensure that the remaining smaller clip is centered.
            }
            break;
        }
    case VideoAlignmentCustom:
    default:
        NIY;
        break;
    }
    ASSERT_MORE_THAN_EQUALS_ZERO(mRegionOfInterest.GetX());
    ASSERT_MORE_THAN_EQUALS_ZERO(mRegionOfInterest.GetY());
    ASSERT_LESS_THAN_EQUALS(mRegionOfInterest.GetWidth(),scaledsize.GetWidth());
    ASSERT_LESS_THAN_EQUALS(mRegionOfInterest.GetHeight(),scaledsize.GetHeight());
    ASSERT_LESS_THAN_EQUALS(mRegionOfInterest.GetWidth(),outputsize.GetWidth());
    ASSERT_LESS_THAN_EQUALS(mRegionOfInterest.GetHeight(),outputsize.GetHeight());

    ////////////////////////////////////////////////////////////////////////////////////

    ProcessEvent(EventChangeVideoClipScaling(mScaling));
    ProcessEvent(EventChangeVideoClipScalingFactor(mScalingFactor));
}

wxRect VideoClip::determineRegionOfInterest(wxSize inputsize, wxSize outputsize, VideoAlignment alignment)
{
    NIY;
    wxSize size(std::min(inputsize.x,outputsize.x),std::min(inputsize.y,outputsize.y)); // todo make min for sizes?

    pixel xdiff = inputsize.x - outputsize.x;
    pixel ydiff = inputsize.y - outputsize.y;

    wxRect roi(wxPoint(0,0),size);

    switch (alignment)
    {
    case VideoAlignmentCenter:
        {
            mAlignmentOffset = wxPoint(0,0);
            if (xdiff >= 0)
            {
                // Input is larger than required size. Reduce input size by setting region of interest.
                roi.SetX(xdiff / 2);
                roi.SetWidth(outputsize.GetWidth());
            }
            else
            {
                // Input is smaller than required size. Entire input is used.
                roi.SetX(0);
                roi.SetWidth(inputsize.GetWidth());
                // Set offset to ensure that the remaining smaller clip is centered.
                mAlignmentOffset.x = -1 * xdiff / 2;
            }
            if (ydiff >= 0)
            {
                // Input is larger than required size. Reduce input size by setting region of interest.
                roi.SetY(ydiff / 2);
                roi.SetHeight(outputsize.GetHeight());
            }
            else
            {
                // Input is smaller than required size. Entire input is used.
                roi.SetY(0);
                roi.SetHeight(inputsize.GetHeight());
                // Set offset to ensure that the remaining smaller clip is centered.
                mAlignmentOffset.y = -1 * ydiff / 2;
            }

            wxSize diff = size - outputsize;
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
    ar & mAlignmentOffset;
    ar & mRegionOfInterest;
}
template void VideoClip::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void VideoClip::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);
} //namespace