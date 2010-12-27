#include "IntervalChange.h"

#include "Intervals.h"
#include "Timeline.h"
#include "UtilLog.h"

namespace gui { namespace timeline { namespace command {

IntervalChange::IntervalChange(gui::timeline::Timeline& timeline, long begin, long end, bool add)
:   ATimelineCommand(timeline)
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

}}} // namespace
