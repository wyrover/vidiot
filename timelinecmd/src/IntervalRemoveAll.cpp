#include "IntervalRemoveAll.h"

#include "Intervals.h"
#include "Timeline.h"
#include "UtilLog.h"
#include "SequenceView.h"

namespace gui { namespace timeline { namespace command {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

IntervalRemoveAll::IntervalRemoveAll(model::SequencePtr sequence)
:   ATimelineCommand(sequence)
,   mIntervals(getTimeline().getIntervals().get())
{
    VAR_INFO(this);
    mCommandName = _("Remove all markers");
}

IntervalRemoveAll::~IntervalRemoveAll()
{
}

//////////////////////////////////////////////////////////////////////////
// COMMAND
//////////////////////////////////////////////////////////////////////////

bool IntervalRemoveAll::Do()
{
    VAR_INFO(this);
    getTimeline().getIntervals().removeAll();
    return true;
}

bool IntervalRemoveAll::Undo()
{
    VAR_INFO(this);
    getTimeline().getIntervals().set(mIntervals);
    return true;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const IntervalRemoveAll& obj )
{
    os << static_cast<const ATimelineCommand&>(obj) << '|' << obj.mIntervals;
    return os;
}

}}} // namespace