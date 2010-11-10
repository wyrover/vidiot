#include "TimelineIntervalRemoveAll.h"

#include "SelectIntervals.h"
#include "GuiTimeLine.h"
#include "UtilLog.h"

namespace command {

TimelineIntervalRemoveAll::TimelineIntervalRemoveAll(gui::timeline::GuiTimeLine& timeline)
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
    mOldRegion = getTimeline().getSelectIntervals().get();
    getTimeline().getSelectIntervals().set(wxRegion());
    return true;
}

bool TimelineIntervalRemoveAll::Undo()
{
    VAR_INFO(this);
    getTimeline().getSelectIntervals().set(mOldRegion);
    return true;
}

} // namespace