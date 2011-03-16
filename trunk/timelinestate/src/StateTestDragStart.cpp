#include "StateTestDragStart.h"

#include "Drag.h"
#include "StateDragging.h"
#include "StateIdle.h"
#include "UtilLog.h"
#include "Tooltip.h"

namespace gui { namespace timeline { namespace state {

const wxString sTooltip = _(
    "Move the cursor to place the currently selected clips somewhere else.");

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TestDragStart::TestDragStart( my_context ctx ) // entry
    :   TimeLineState( ctx )
    ,   mStartPosition(0,0)
{
    LOG_DEBUG; 

    const EvLeftDown* event = dynamic_cast<const EvLeftDown*>(triggering_event());
    ASSERT(event); // Only way to get here is to press left button in the Idle state

    mStartPosition = event->mPosition;
}

TestDragStart::~TestDragStart() // exit
{ 
    LOG_DEBUG; 
}
//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result TestDragStart::react( const EvLeftUp& evt )
{
    VAR_DEBUG(evt);
    return transit<Idle>();
}

boost::statechart::result TestDragStart::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);
    wxPoint diff = mStartPosition - evt.mPosition;
    static int tolerance = 2;
    if ((abs(diff.x) > tolerance) || (abs(diff.y) > tolerance))
    {
        getDrag().start(evt.mPosition, false);
        return transit<Dragging>();
    }
    return forward_event();
}

boost::statechart::result TestDragStart::react( const EvLeave& evt )
{
    VAR_DEBUG(evt);
    return transit<Idle>();
}

boost::statechart::result TestDragStart::react( const EvKeyDown& evt)
{
    VAR_DEBUG(evt);
    switch (evt.mWxEvent.GetKeyCode())
    {
    case WXK_F1:
        getTooltip().show(sTooltip);
        break;
    }
    return forward_event();
}

}}} // namespace