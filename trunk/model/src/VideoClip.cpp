#include "VideoClip.h"

#include <boost/make_shared.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "UtilLog.h"
#include "VideoFile.h"

namespace model {

VideoClip::VideoClip()
    :	Clip()
    ,   mProgress(0)
{
    VAR_DEBUG(this);
}

VideoClip::VideoClip(VideoFilePtr file)
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
// IVIDEO
//////////////////////////////////////////////////////////////////////////

VideoFramePtr VideoClip::getNextVideo(int requestedWidth, int requestedHeight, bool alpha)
{
    if (getLastSetPosition())
    {
        mProgress = *getLastSetPosition(); // Reinitialize mProgress to the last value set in ::moveTo
        invalidateLastSetPosition();
    }

    unsigned int length = getNumberOfFrames();

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
            NIY; // NIY since I did not have a file for testing yet.
        }
    }

    VAR_VIDEO(videoFrame);
    setGenerationProgress(mProgress);
    return videoFrame;
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
