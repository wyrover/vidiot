#include "TimelineIntervalChange.h"
#include "UtilLog.h"

namespace command {

    TimelineIntervalChange::TimelineIntervalChange(gui::timeline::IntervalsPtr intervals, long begin, long end, bool add)
    :   TimelineCommand()
    ,   mIntervals(intervals)
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
    mIntervals->change(mBegin,mEnd,mAdd);
    return true;
}

bool TimelineIntervalChange::Undo()
{
    VAR_INFO(this);
    mIntervals->change(mBegin,mEnd,!mAdd);
    return true;
}

} // namespace