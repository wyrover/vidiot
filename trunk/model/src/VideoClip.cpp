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

const int sScalingOriginalSize = Convert::factorToDigits(1,Constants::scalingPrecision);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoClip::VideoClip()
    : Clip()
    , mProgress(0)
    , mScaling()
    , mScalingFactor(sScalingOriginalSize)
    , mAlignment()
    , mPosition(0,0)
{
    VAR_DEBUG(this);
}

VideoClip::VideoClip(IControlPtr file)
    : Clip(file)
    , mProgress(0)
    , mScaling(Config::ReadEnum<VideoScaling>(Config::sPathDefaultVideoScaling))
    , mScalingFactor(sScalingOriginalSize)
    , mAlignment(Config::ReadEnum<VideoAlignment>(Config::sPathDefaultVideoAlignment))
    , mPosition(0,0)
{
    VAR_DEBUG(this);
    updateAutomatedScaling();
    updateAutomatedPositioning();
}

VideoClip::VideoClip(const VideoClip& other)
    : Clip(other)
    , mProgress(0)
    , mScaling(other.mScaling)
    , mScalingFactor(other.mScalingFactor)
    , mAlignment(other.mAlignment)
    , mPosition(other.mPosition)
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
        wxSize requiredOutputSize = Convert::sizeInBoundingBox(outputsize, size, scaleToBoundingBox);
        ASSERT_NONZERO(scaleToBoundingBox);
        VideoFilePtr generator = getDataGenerator<VideoFile>();
        double videoscaling = getScalingFactorDouble() * scaleToBoundingBox;
        wxSize inputsize = generator->getSize();
        wxSize requiredVideoSize = Convert::scale(inputsize, videoscaling);

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

            wxSize inputsize = getInputSize();
            wxSize outputsize = Properties::get()->getVideoSize();
            wxSize scaledsize = Convert::scale(inputsize,getScalingFactorDouble());
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

wxSize VideoClip::getSize() const
{
    // Without scaling: use default project size
    // Todo add scaling (make smaller) for compositing
    // todo  check if obsolete first?
    return Properties::get()->getVideoSize();
}

VideoScaling VideoClip::getScaling() const
{
    return mScaling;
}

int VideoClip::getScalingFactor() const // todo rename into getscalingvalue and the other into getscalingfactor
{
    return mScalingFactor;
}

double VideoClip::getScalingFactorDouble() const
{
    return Convert::digitsToFactor(mScalingFactor,Constants::scalingPrecision);
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
    wxSize scaledsize = Convert::scale(inputsize,getScalingFactorDouble());
    return wxPoint(-1 * scaledsize.x, -1 * scaledsize.y);
}

wxPoint VideoClip::getMaxPosition()
{
    wxSize outputsize = Properties::get()->getVideoSize();
    return wxPoint(outputsize.x,outputsize.y);
}

void VideoClip::setScaling(VideoScaling scaling, boost::optional<int> factor)
{
    VideoScaling oldScaling = mScaling;
    int oldScalingFactor = mScalingFactor;
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
    wxSize outputsize = Properties::get()->getVideoSize();

    switch (mScaling)
    {
    case VideoScalingFitToFill:
        {
            double scalingfactor;
            Convert::fillBoundingBoxWithMinimalLoss(inputsize, outputsize, scalingfactor);
            mScalingFactor = Convert::factorToDigits(scalingfactor,Constants::scalingPrecision);
            break;
        }
    case VideoScalingFitAll:
        {
            double scalingfactor;
            Convert::sizeInBoundingBox(inputsize, outputsize, scalingfactor);
            mScalingFactor = Convert::factorToDigits(scalingfactor,Constants::scalingPrecision);
            break;
        }
    case VideoScalingNone:
        {
            mScalingFactor = sScalingOriginalSize;
            break;
        }
    case VideoScalingCustom:
    default:
        // Do not automatically determine scaling factor
        break;
    }

    ASSERT_LESS_THAN_EQUALS(mScalingFactor,sScalingOriginalSize);
    ASSERT_MORE_THAN_ZERO(mScalingFactor);
}

void VideoClip::updateAutomatedPositioning()
{
    wxSize inputsize = getInputSize();
    wxSize scaledsize = Convert::scale(inputsize,getScalingFactorDouble());
    wxSize outputsize = Properties::get()->getVideoSize();
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
    os << static_cast<const Clip&>(obj) << '|' << obj.mProgress << '|' << obj.mScaling << '|' << obj.mScalingFactor << '|' << obj.mAlignment << '|' << obj.mPosition;
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
    ar & mPosition;
}
template void VideoClip::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void VideoClip::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);
} //namespace