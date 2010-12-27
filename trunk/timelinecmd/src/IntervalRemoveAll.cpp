#include "TimelineIntervalRemoveAll.h"

#include "Intervals.h"
#include "Timeline.h"
#include "UtilLog.h"

namespace command {

TimelineIntervalRemoveAll::TimelineIntervalRemoveAll(gui::timeline::Timeline& timeline)
:   TimelineCommand(timeline)
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
    mOldRegion = getTimeline().getIntervals().get();
    getTimeline().getIntervals().set(wxRegion());
    return true;
}

bool TimelineIntervalRemoveAll::Undo()
{
    VAR_INFO(this);
    getTimeline().getIntervals().set(mOldRegion);
    return true;
}

} // namespace