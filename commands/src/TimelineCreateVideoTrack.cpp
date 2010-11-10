#include "TimelineCreateVideoTrack.h"

#include <boost/make_shared.hpp>
#include "UtilLog.h"

namespace command {

TimelineCreateVideoTrack::TimelineCreateVideoTrack(gui::timeline::GuiTimeLine& timeline, model::SequencePtr sequence)
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
    mSequence->addVideoTrack(mNewTrack);
    return true;
}

bool TimelineCreateVideoTrack::Undo()
{
    VAR_INFO(this);
    mSequence->removeVideoTrack(mNewTrack);
    return true;
}

} // namespace