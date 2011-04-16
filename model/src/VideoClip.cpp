#include "VideoClip.h"

#include <boost/make_shared.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "UtilLog.h"
#include "VideoFile.h"
#include "EmptyFrame.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoClip::VideoClip()
    :	Clip()
    ,   mProgress(0)
{
    VAR_DEBUG(this);
}

VideoClip::VideoClip(IControlPtr file)
    :	Clip(file)
    ,   mProgress(0)
{
    VAR_DEBUG(this);
}

VideoClip::VideoClip(const VideoClip& other)
    :   Clip(other)
    ,   mProgress(0)
{
    VAR_DEBUG(*this);
}

VideoClip* VideoClip::clone()
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

VideoFramePtr VideoClip::getNextVideo(int requestedWidth, int requestedHeight, bool alpha)
{
    if (getLastSetPosition())
    {
        mProgress = *getLastSetPosition(); // Reinitialize mProgress to the last value set in ::moveTo
        invalidateLastSetPosition();
    }

    unsigned int length = getLength();

    VideoFramePtr videoFrame;

    if (mProgress < length)
    {
        videoFrame = getDataGenerator<VideoFile>()->getNextVideo(requestedWidth, requestedHeight, alpha);
        if (videoFrame)
        {
            ASSERT(videoFrame->getRepeat() > 0);
            if (mProgress + videoFrame->getRepeat() > length)
            {
                videoFrame->setRepeat(length - mProgress);
                mProgress = length;
            }
            else
            {
                mProgress += videoFrame->getRepeat();
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

            PixelFormat format = PIX_FMT_RGBA;
            if (!alpha)
            {
                format = PIX_FMT_RGB24;
            }

            /** todo timebase. See also EmptyFile.cpp::getnextvideo */
            AVRational timebase;
            timebase.num = 0;
            timebase.den = 0;
            videoFrame = boost::static_pointer_cast<VideoFrame>(boost::make_shared<EmptyFrame>(format, requestedWidth, requestedHeight, mProgress, timebase));

            // @todo use repeat in case multiple extra video images are needed?
            mProgress += 1;

        }
    }

    VAR_VIDEO(videoFrame);
    setGenerationProgress(mProgress);
    return videoFrame;
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
