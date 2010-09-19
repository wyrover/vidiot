#include "TimelineIntervalRemoveAll.h"

#include "SelectIntervals.h"
#include "UtilLog.h"

namespace command {

TimelineIntervalRemoveAll::TimelineIntervalRemoveAll(gui::timeline::IntervalsPtr intervals)
:   TimelineCommand()
,   mIntervals(intervals)
{
    VAR_INFO(this);
    mCommandName = _("Remove all markers");
}

TimelineIntervalRemoveAll::~TimelineIntervalRemoveAll()
{
}

bool TimelineIntervalRemoveAll::Do()
{
    VAR_INFO(this);
    mOldRegion = mIntervals->get();
    mIntervals->set(wxRegion());
    return true;
}

bool TimelineIntervalRemoveAll::Undo()
{
    VAR_INFO(this);
    mIntervals->set(mOldRegion);
    return true;
}

} // namespace