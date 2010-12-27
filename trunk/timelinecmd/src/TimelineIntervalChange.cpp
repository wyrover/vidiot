#include "TimelineIntervalChange.h"

#include "Intervals.h"
#include "Timeline.h"
#include "UtilLog.h"

namespace command {

TimelineIntervalChange::TimelineIntervalChange(gui::timeline::Timeline& timeline, long begin, long end, bool add)
:   TimelineCommand(timeline)
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

TimelineIntervalChange::~TimelineIntervalChange()
{
}

bool TimelineIntervalChange::Do()
{
    VAR_INFO(this);
    getTimeline().getIntervals().change(mBegin,mEnd,mAdd);
    return true;
}

bool TimelineIntervalChange::Undo()
{
    VAR_INFO(this);
    getTimeline().getIntervals().change(mBegin,mEnd,!mAdd);
    return true;
}

} // namespace