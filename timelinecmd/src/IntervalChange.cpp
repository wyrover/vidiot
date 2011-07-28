#include "IntervalChange.h"

#include "Intervals.h"
#include "Timeline.h"
#include "UtilLog.h"

namespace gui { namespace timeline { namespace command {

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

    IntervalChange::IntervalChange(model::SequencePtr sequence, long begin, long end, bool add)
:   ATimelineCommand(sequence)
,   mBegin(begin)
,   mEnd(end)
,   mAdd(add)
{
    VAR_INFO(this);
    if (mAdd)
    {
        mCommandName = _("Add new interval selection");
    }
    else
    {
        mCommandName = _("Remove new interval selection");
    }
}

IntervalChange::~IntervalChange()
{
}

//////////////////////////////////////////////////////////////////////////
// COMMAND
//////////////////////////////////////////////////////////////////////////

bool IntervalChange::Do()
{
    VAR_INFO(this);
    getTimeline().getIntervals().change(mBegin,mEnd,mAdd);
    return true;
}

bool IntervalChange::Undo()
{
    VAR_INFO(this);
    getTimeline().getIntervals().change(mBegin,mEnd,!mAdd);
    return true;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const IntervalChange& obj )
{
    os << static_cast<const ATimelineCommand&>(obj) << '|' << obj.mBegin << '|' << obj.mEnd << '|' << obj.mAdd;
    return os;
}

}}} // namespace
