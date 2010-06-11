#include "VideoTrack.h"

#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "UtilLog.h"
#include "AProjectViewNode.h"
#include "VideoClip.h"

namespace model {

VideoTrack::VideoTrack()
:	Track()
,   mPts(0)
{ 
    VAR_DEBUG(this);

    mItClips = mClips.begin();
}

VideoTrack::~VideoTrack()
{
    VAR_DEBUG(this);
}

void VideoTrack::moveTo(boost::int64_t position)
{
    Track::moveTo(position);
    mPts = position;
}


void VideoTrack::addVideoClip(VideoClipPtr clip)
{
    mClips.push_back(clip);
}

void VideoTrack::removeVideoClip(VideoClipPtr clip)
{
    NIY
}

VideoFramePtr VideoTrack::getNextVideo(int requestedWidth, int requestedHeight, bool alpha)
{
    VideoFramePtr videoFrame = VideoFramePtr();

    while (!videoFrame && mItClips != mClips.end())
    {
        videoFrame = boost::static_pointer_cast<model::VideoClip>(*mItClips)->getNextVideo(requestedWidth, requestedHeight, alpha);
        if (!videoFrame)
        {
            mItClips++;
            if (mItClips != mClips.end())
            {
                (*mItClips)->moveTo(0);
            }
        }
    }
    if (videoFrame)
    {
        videoFrame->setPts(mPts++);
    }
    VAR_VIDEO(videoFrame);
    return videoFrame;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void VideoTrack::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<Track>(*this);
    ar & boost::serialization::base_object<IVideo>(*this);
}
template void VideoTrack::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void VideoTrack::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace
