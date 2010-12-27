#include "StateMovingCursor.h"

#include "StateIdle.h"
#include "UtilLog.h"
#include "Cursor.h"
#include "Intervals.h"
#include "Tooltip.h"

namespace gui { namespace timeline { namespace state {

const wxString sTooltip = _("Move the cursor to 'scrub' over the timeline and see the frames back in the preview.");

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

MovingCursor::MovingCursor( my_context ctx ) // entry
:   TimeLineState( ctx )
,   mToggling(false)
{
    LOG_DEBUG; 
}

MovingCursor::~MovingCursor() // exit
{ 
    LOG_DEBUG; 
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result MovingCursor::react( const EvLeftDown& evt )
{
    VAR_DEBUG(evt);
    getCursor().moveCursorOnUser(evt.mPosition.x);
    if (evt.mWxEvent.ShiftDown())
    {
        triggerToggleStart();
    }
    return discard_event();
}

boost::statechart::result MovingCursor::react( const EvLeftUp& evt )
{
    VAR_DEBUG(evt);
    if (evt.mWxEvent.ShiftDown())
    {
        triggerToggleEnd();
    }
    return transit<Idle>();
}

boost::statechart::result MovingCursor::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);
    getCursor().moveCursorOnUser(evt.mPosition.x); // Will also update the 'running selection' 
    return discard_event();
}

boost::statechart::result MovingCursor::react( const EvKeyDown& evt)
{
    VAR_DEBUG(evt);
    switch (evt.mWxEvent.GetKeyCode())
    {
    case WXK_SHIFT:
        triggerToggleStart();
        break;
    case WXK_F1:
        getTooltip().show(sTooltip);
        break;
    }
    return discard_event();
}

boost::statechart::result MovingCursor::react( const EvKeyUp& evt)
{
    VAR_DEBUG(evt);
    switch (evt.mWxEvent.GetKeyCode())
    {
    case WXK_SHIFT:
        triggerToggleEnd();
        break;
    }
    return discard_event();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void MovingCursor::triggerToggleStart()
{
    if (!mToggling)
    {
        getIntervals().startToggle();
        mToggling = true;
    }
}

void MovingCursor::triggerToggleEnd()
{
    if (mToggling)
    {
        getIntervals().endToggle();
        mToggling = false;
    }
}

}}} // namespace