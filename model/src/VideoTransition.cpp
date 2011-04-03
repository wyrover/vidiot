#include "VideoTransition.h"

#include <boost/make_shared.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include "UtilLog.h"

namespace model {

VideoTransition::VideoTransition()
    :	Transition()
    ,   IVideo()
    ,   mPosition(-1)
{
    VAR_DEBUG(this);
}

VideoTransition::VideoTransition(pts nFramesLeft, pts nFramesRight)
:   Transition(nFramesLeft,nFramesRight)
,   IVideo()
,   mPosition(-1)
{
    VAR_DEBUG(this);
}

VideoTransition::VideoTransition(const VideoTransition& other)
:   Transition(other)
,   IVideo()
,   mPosition(-1)
{
    VAR_DEBUG(*this);
}

VideoTransition* VideoTransition::clone()
{ 
    return new VideoTransition(static_cast<const VideoTransition&>(*this)); 
}

VideoTransition::~VideoTransition()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// IVIDEO
//////////////////////////////////////////////////////////////////////////

VideoFramePtr VideoTransition::getNextVideo(int requestedWidth, int requestedHeight, bool alpha)
{
    if (getLastSetPosition())
    {
        mPosition = *getLastSetPosition(); // Reinitialize mProgress to the last value set in ::moveTo
        invalidateLastSetPosition();

        // todo initialize previous and next clips here
        //mLeft->moveTo(mLeft->getLength() - mFramesLeft + position);
        //mRight->moveTo(position);


    }


    VideoFramePtr videoFrame;
    return videoFrame;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const VideoTransition& obj )
{
    os << static_cast<const Transition&>(obj) << '|' << obj.mPosition;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void VideoTransition::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<Transition>(*this);
    ar & boost::serialization::base_object<IVideo>(*this);
}
template void VideoTransition::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void VideoTransition::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace
