#include "StateLeftDown.h"

#include "Drag.h"
#include "EventDrag.h"
#include "EventKey.h"
#include "EventMouse.h"
#include "EventPart.h"
#include "Layout.h"
#include "Mouse.h"
#include "StateDragging.h"
#include "StateIdle.h"
#include "Tooltip.h"

#include "UtilLog.h"

namespace gui { namespace timeline { namespace state {

const wxString StateLeftDown::sTooltip = _(
    "Move the cursor to place the currently selected clips somewhere else."
    );

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

StateLeftDown::StateLeftDown( my_context ctx ) // entry
    :   TimeLineState( ctx )
    ,   mStartPosition(getMouse().getLeftDownPosition())
{
    LOG_DEBUG;
}

StateLeftDown::~StateLeftDown() // exit
{
    LOG_DEBUG;
}
//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result StateLeftDown::react( const EvLeftDown& evt )
{
    VAR_DEBUG(evt);
    FATAL("Not allowed");
    return discard_event();
}

boost::statechart::result StateLeftDown::react( const EvLeftUp& evt )
{
    VAR_DEBUG(evt);
    return transit<Idle>();
}

boost::statechart::result StateLeftDown::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);
    wxPoint diff = mStartPosition - getMouse().getVirtualPosition();
    if ((abs(diff.x) > Layout::DragThreshold) || (abs(diff.y) > Layout::DragThreshold))
    {
        getDrag().start(getMouse().getVirtualPosition(), true);
        return transit<Dragging>();
    }
    return forward_event();
}

boost::statechart::result StateLeftDown::react( const EvLeave& evt )
{
    VAR_DEBUG(evt);
    return transit<Idle>();
}

boost::statechart::result StateLeftDown::react( const EvKeyDown& evt)
{
    VAR_DEBUG(evt);
    switch (evt.getKeyCode())
    {
    case WXK_F1:
        getTooltip().show(sTooltip);
        break;
    case WXK_ESCAPE:
        return transit<Idle>();
    }
    return forward_event();
}

}}} // namespace