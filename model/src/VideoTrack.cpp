#include "VideoTrack.h"

#include <boost/make_shared.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "UtilLog.h"
#include "VideoClip.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoTrack::VideoTrack()
:	Track()
,   mPts(0)
{ 
    VAR_DEBUG(this);
}

VideoTrack::VideoTrack(const VideoTrack& other)
:   Track(other)
,   mPts(0)
{
    VAR_DEBUG(this);
}

VideoTrack* VideoTrack::clone()
{ 
    return new VideoTrack(static_cast<const VideoTrack&>(*this)); 
}

VideoTrack::~VideoTrack()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

void VideoTrack::clean()
{
    VAR_DEBUG(this);
    mPts = 0;
    Track::clean();
}

//////////////////////////////////////////////////////////////////////////
// PLAYBACK
//////////////////////////////////////////////////////////////////////////

void VideoTrack::moveTo(pts position)
{
    Track::moveTo(position);
    mPts = position;
}

VideoFramePtr VideoTrack::getNextVideo(int requestedWidth, int requestedHeight, bool alpha)
{
    VideoFramePtr videoFrame;

    while (!videoFrame && iterate_hasClip())
    {
        videoFrame = boost::dynamic_pointer_cast<IVideo>(iterate_getClip())->getNextVideo(requestedWidth, requestedHeight, alpha);
        if (!videoFrame)
        {
            iterate_nextClip();
            if (iterate_hasClip())
            {
                iterate_getClip()->moveTo(0);
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
