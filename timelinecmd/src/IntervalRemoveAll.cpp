#include "IntervalRemoveAll.h"

#include "Intervals.h"
#include "Timeline.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"

namespace gui { namespace timeline { namespace command {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

IntervalRemoveAll::IntervalRemoveAll(model::SequencePtr sequence)
:   ATimelineCommand(sequence)
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

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const IntervalRemoveAll& obj )
{
    os << static_cast<const ATimelineCommand&>(obj) << '|' << obj.mOldRegion;
    return os;
}

}}} // namespace
