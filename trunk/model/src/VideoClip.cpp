#include "VideoClip.h"

#include <boost/make_shared.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "UtilLog.h"
#include "AProjectViewNode.h"
#include "VideoFile.h"

namespace model {


VideoClip::VideoClip()
    :	Clip()
{
    VAR_DEBUG(this);
}

VideoClip::VideoClip(VideoFilePtr file)
    :	Clip(file)
{
    VAR_DEBUG(this);
}

VideoClip::~VideoClip()
{
    VAR_DEBUG(this);
}

VideoFramePtr VideoClip::getNextVideo(int requestedWidth, int requestedHeight, bool alpha)
{
    VideoFramePtr videoFrame = boost::static_pointer_cast<VideoFile>(mRender)->getNextVideo(requestedWidth, requestedHeight, alpha);
    VAR_VIDEO(videoFrame);
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
