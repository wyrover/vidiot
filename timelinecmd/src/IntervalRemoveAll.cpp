#include "IntervalRemoveAll.h"

#include "Intervals.h"
#include "Timeline.h"
#include "UtilLog.h"

namespace gui { namespace timeline { namespace command {

IntervalRemoveAll::IntervalRemoveAll(gui::timeline::Timeline& timeline)
:   ATimelineCommand(timeline)
{
    VAR_INFO(this);
    mCommandName = _("Remove all markers");
}

IntervalRemoveAll::~IntervalRemoveAll()
{
}

bool IntervalRemoveAll::Do()
{
    VAR_INFO(this);
    mOldRegion = getTimeline().getIntervals().get();
    getTimeline().getIntervals().set(wxRegion());
    return true;
}

bool IntervalRemoveAll::Undo()
{
    VAR_INFO(this);
    getTimeline().getIntervals().set(mOldRegion);
    return true;
}

}}} // namespace
