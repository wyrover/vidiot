#include "StateMovingCursor.h"

#include "GuiTimeLine.h"
#include "StateIdle.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"

namespace gui { namespace timeline { namespace state {

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
    getTimeline().moveCursorOnUser(evt.mPosition.x);
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
    getTimeline().moveCursorOnUser(evt.mPosition.x); // Will also update the 'running selection' 
    return discard_event();
}

boost::statechart::result MovingCursor::react( const EvKeyDown& evt)
{
    switch (evt.mWxEvent.GetKeyCode())
    {
    case WXK_SHIFT:
        triggerToggleStart();
        break;
    }
    return discard_event();
}

boost::statechart::result MovingCursor::react( const EvKeyUp& evt)
{
    switch (evt.mWxEvent.GetKeyCode())
    {
    case WXK_SHIFT:
        triggerToggleEnd();
        break;
    }
    return discard_event();
}

////////////////////////////////////////////////////////////////////////////
//// MAKE IT A PART
////////////////////////////////////////////////////////////////////////////
//
//GuiTimeLine& MovingCursor::getTimeline()
//{
//    return outermost_context().mTimeline;
//}
//
//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void MovingCursor::triggerToggleStart()
{
    if (!mToggling)
    {
        getSelectIntervals().startToggle();
        mToggling = true;
    }
}

void MovingCursor::triggerToggleEnd()
{
    if (mToggling)
    {
        getSelectIntervals().endToggle();
        mToggling = false;
    }
}

}}} // namespace