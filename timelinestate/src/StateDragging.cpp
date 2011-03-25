#include "StateDragging.h"

#include <wx/utils.h>
#include "Drag.h"
#include "UtilLog.h"
#include "StateIdle.h"
#include "Tooltip.h"
#include "Timeline.h"

namespace gui { namespace timeline { namespace state {

    const wxString sTooltip = _(
        "Move the clips by dragging them around.\n" \
        "Release Left Mouse Button to 'drop'.\n\n" \
        "CTRL: Disable snapping and shifting of clips\n" \
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
    // See also EvDragDrop
    VAR_DEBUG(evt);
    getDrag().drop();
    getDrag().stop();
    return transit<Idle>();
}

boost::statechart::result Dragging::react( const EvDragDrop& evt )
{
    // See also EvLeftUp
    // TODO why so difficult? This is because there are no events generated during drag&drop. Isn't it easier to make (using regular mouse events?)
    VAR_DEBUG(evt);
    getDrag().drop();
    getDrag().stop();
    return transit<Idle>();
}

boost::statechart::result Dragging::react( const EvMotion& evt )
{
    // See also EvDragMove
    VAR_DEBUG(evt);
    getDrag().move(evt.mPosition, evt.mWxEvent.ControlDown(), evt.mWxEvent.ShiftDown());
    return forward_event();
}

boost::statechart::result Dragging::react( const EvDragMove& evt )
{
    // See also EvMotion
    VAR_DEBUG(evt);
    getDrag().move(evt.mPosition,wxGetMouseState().ControlDown(),wxGetMouseState().ShiftDown());
    return forward_event();
}

boost::statechart::result Dragging::react( const EvLeave& evt )
{
    // See also EvDragEnd
    VAR_DEBUG(evt);
    getDrag().stop();
    return transit<Idle>();
}

boost::statechart::result Dragging::react( const EvDragEnd& evt )
{
    // See also EvLeave
    VAR_DEBUG(evt);
    getDrag().stop();
    return transit<Idle>();
}

boost::statechart::result Dragging::react( const EvKeyDown& evt )
{
    VAR_DEBUG(evt);

    switch (evt.mWxEvent.GetKeyCode())
    {
    case WXK_ALT:
    case WXK_SHIFT:
        getDrag().move(evt.mPosition, evt.mWxEvent.ControlDown(), evt.mWxEvent.ShiftDown());
        break;
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
    case WXK_ALT:
    case WXK_SHIFT:
        getDrag().move(evt.mPosition, evt.mWxEvent.ControlDown(), evt.mWxEvent.ShiftDown());
        break;
    }
    return forward_event();
}

}}} // namespace