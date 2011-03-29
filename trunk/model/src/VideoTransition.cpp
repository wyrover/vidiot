#include "VideoTransition.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "Clip.h"
#include "AProjectViewNode.h"
#include "Convert.h"
#include "Project.h"
#include "Properties.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

VideoTransition::VideoTransition()
:	IControl()
,   mClips()
,   mLength(0)
{
    VAR_DEBUG(*this);
}

VideoTransition::VideoTransition(Clips clips, pts length)
:	IControl()
,   mClips(clips)
,   mLength(length)
{
    VAR_DEBUG(*this);
}

VideoTransition::VideoTransition(const VideoTransition& other)
:   IControl()
,   mClips(other.mClips)
,   mLength(other.mLength)
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
// ICONTROL
//////////////////////////////////////////////////////////////////////////

pts VideoTransition::getLength()
{
    return mLength;
}

void VideoTransition::moveTo(pts position)
{
    NIY
}

wxString VideoTransition::getDescription() const
{
    return _("Transition");
}

void VideoTransition::clean()
{
    NIY
}

//////////////////////////////////////////////////////////////////////////
// IVIDEO
//////////////////////////////////////////////////////////////////////////

VideoFramePtr VideoTransition::getNextVideo(int requestedWidth, int requestedHeight, bool alpha)
{
    VideoFramePtr frame;
    return frame;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const VideoTransition& obj )
{
    os << obj.mClips << '|' << obj.mLength;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void VideoTransition::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<IControl>(*this);
}
template void VideoTransition::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void VideoTransition::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace
