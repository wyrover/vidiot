#include "StateDragging.h"

#include "Drag.h"
#include "Drop.h"
#include "UtilLog.h"
#include "StateIdle.h"
#include "Tooltip.h"
#include "Timeline.h"

namespace gui { namespace timeline { namespace state {

    const wxString sTooltip = _(
        "Move the clips by dragging them around.\n" \
        "Release Left Mouse Button to 'drop'.\n\n" \
        "CTRL: Disable snapping\n" \
        "SHIFT: Change 'grab point'\n" \
        );

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

/** /todo handle mouse focus lost */

Dragging::Dragging( my_context ctx ) // entry
    :   TimeLineState( ctx )
{
    LOG_DEBUG; 
}

Dragging::~Dragging() // exit
{ 
    LOG_DEBUG; 
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result Dragging::react( const EvLeftUp& evt )
{
    VAR_DEBUG(evt);

    getDrag().drop();
    getDrag().stop();

    return transit<Idle>();
}

boost::statechart::result Dragging::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);

    getDrag().move(evt.mPosition, evt.mWxEvent.ShiftDown());
    getDrop().updateDropArea(evt.mPosition);

    return forward_event();
}

boost::statechart::result Dragging::react( const EvLeave& evt )
{
    VAR_DEBUG(evt);
    getDrag().stop();
    return transit<Idle>();
}

boost::statechart::result Dragging::react( const EvKeyDown& evt )
{
    VAR_DEBUG(evt);

    switch (evt.mWxEvent.GetKeyCode())
    {
    case WXK_F1:
        getTooltip().show(sTooltip);
        break;
    case WXK_ESCAPE:
        getDrag().stop();
        return transit<Idle>();
    }
    return forward_event();
}

boost::statechart::result Dragging::react( const EvKeyUp& evt )
{
    VAR_DEBUG(evt);

    switch (evt.mWxEvent.GetKeyCode())
    {
    case WXK_SHIFT:
        getDrag().move(evt.mPosition, false);
        break;
    }
    return forward_event();
}

}}} // namespace