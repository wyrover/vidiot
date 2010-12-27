#include "TimelineCreateVideoTrack.h"

#include <boost/make_shared.hpp>
#include <boost/assign/list_of.hpp>
#include "UtilLog.h"

namespace command {

TimelineCreateVideoTrack::TimelineCreateVideoTrack(gui::timeline::Timeline& timeline, model::SequencePtr sequence)
:   TimelineCommand(timeline)
,   mSequence(sequence)
,   mNewTrack()
{
    VAR_INFO(this)(mSequence);
    mCommandName = _("Add track"); 
}

TimelineCreateVideoTrack::~TimelineCreateVideoTrack()
{
}

bool TimelineCreateVideoTrack::Do()
{
    VAR_INFO(this);
    if (!mNewTrack)
    {
        mNewTrack = boost::make_shared<model::VideoTrack>();
    }
    mSequence->addVideoTracks(boost::assign::list_of(mNewTrack));
    return true;
}

bool TimelineCreateVideoTrack::Undo()
{
    VAR_INFO(this);
    mSequence->removeVideoTracks(boost::assign::list_of(mNewTrack));
    return true;
}

} // namespace