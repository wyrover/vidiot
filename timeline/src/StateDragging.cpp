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

    getDrag().Stop();

    return transit<Idle>();
}

boost::statechart::result Dragging::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);

    getDrag().MoveTo(evt.mPosition, evt.mWxEvent.ShiftDown());
    getDrop().updateDropArea(evt.mPosition);

    return discard_event();
}

boost::statechart::result Dragging::react( const EvKeyDown& evt )
{
    switch (evt.mWxEvent.GetKeyCode())
    {
    case WXK_F1:
        getTooltip().show(sTooltip);
        break;
    }
    return discard_event();
}

boost::statechart::result Dragging::react( const EvKeyUp& evt )
{
    switch (evt.mWxEvent.GetKeyCode())
    {
    case WXK_SHIFT:
        getDrag().MoveTo(evt.mPosition, false);
        break;
    }
    return discard_event();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Dragging::showDropArea(wxPoint p)
{
}

}}} // namespace