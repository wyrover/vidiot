#include "VideoTransitionClip.h"

#include <boost/make_shared.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "UtilLog.h"
#include "VideoTransition.h"

namespace model {

VideoTransitionClip::VideoTransitionClip()
    :	VideoClip()
{
    VAR_DEBUG(this);
}

VideoTransitionClip::VideoTransitionClip(VideoTransitionPtr transition)
    :	VideoClip(transition)
{
    VAR_DEBUG(this);
}

VideoTransitionClip::VideoTransitionClip(const VideoTransitionClip& other)
    :   VideoClip()
{
    VAR_DEBUG(*this);
}

VideoTransitionClip* VideoTransitionClip::clone()
{ 
    return new VideoTransitionClip(static_cast<const VideoTransitionClip&>(*this)); 
}

VideoTransitionClip::~VideoTransitionClip()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const VideoTransitionClip& obj )
{
    os << static_cast<const VideoClip&>(obj);
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void VideoTransitionClip::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<VideoClip>(*this);
}
template void VideoTransitionClip::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void VideoTransitionClip::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace
