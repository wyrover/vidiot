#include "StateTrim.h"

#include "EventDrag.h"
#include "EventKey.h"
#include "EventMouse.h"
#include "EventPart.h"
#include "Keyboard.h"
#include "StateIdle.h"
#include "Tooltip.h"
#include "UtilLog.h"
#include "Trim.h"

namespace gui { namespace timeline { namespace state {

const wxString sTooltip = _(
    "Move the cursor to change the entry point of the clip.");

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

StateTrim::StateTrim( my_context ctx ) // entry
    :   TimeLineState( ctx )
    ,   mShiftDown(getKeyboard().getShiftDown())
{
    getTrim().start();
}

StateTrim::~StateTrim() // exit
{
    getTrim().stop();
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result StateTrim::react( const EvLeftUp& evt )
{
    VAR_DEBUG(evt);
    getTrim().submit();
    return transit<Idle>();
}

boost::statechart::result StateTrim::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);
    getTrim().update(evt.mWxEvent.GetPosition());
    return forward_event();
}

boost::statechart::result StateTrim::react( const EvLeave& evt )
{
    VAR_DEBUG(evt);
    return transit<Idle>();
}

boost::statechart::result StateTrim::react( const EvKeyDown& evt)
{
    VAR_DEBUG(evt);
    switch (evt.mWxEvent.GetKeyCode())
    {
    case WXK_F1:
        getTooltip().show(sTooltip);
        break;
    case WXK_ESCAPE:
        return transit<Idle>();
    case WXK_SHIFT:
        if (!mShiftDown) // Avoid quirky feedback: when shift dragging, every motion event is followed by a key event. Updating on those key events causes flickering.
        {
            mShiftDown = true;
            getTrim().update(evt.mWxEvent.GetPosition());
            break;
        }
    }
    return forward_event();
}

boost::statechart::result StateTrim::react( const EvKeyUp& evt)
{
    VAR_DEBUG(evt);
    switch (evt.mWxEvent.GetKeyCode())
    {
    case WXK_SHIFT:
        if (mShiftDown) // Avoid quirky feedback: when shift dragging, every motion event is followed by a key event. Updating on those key events causes flickering.
        {
            mShiftDown = false;
            getTrim().update(evt.mWxEvent.GetPosition());
            break;
        }
    }
    return forward_event();
}

}}} // namespace