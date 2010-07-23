#include "StateMovingCursor.h"

//#include <boost/foreach.hpp>
//#include <boost/make_shared.hpp>
//#include "GuiTimeLineClip.h"
//#include "MousePointer.h"
//#include "Track.h"
#include "GuiTimeLine.h"
#include "StateIdle.h"
//#include "GuiTimeLineTrack.h"
//#include "EmptyClip.h"
//#include "Project.h"
//#include "TimelineMoveClips.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"

namespace gui { namespace timeline { namespace state {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

MovingCursor::MovingCursor( my_context ctx ) // entry
:   my_base( ctx )
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
    outermost_context().timeline.moveCursorOnUser(evt.mPosition.x);
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
    outermost_context().timeline.moveCursorOnUser(evt.mPosition.x); // Will also update the 'running selection' 
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

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void MovingCursor::triggerToggleStart()
{
    if (!mToggling)
    {
        outermost_context().timeline.mSelectedIntervals->startToggle();
        mToggling = true;
    }
}

void MovingCursor::triggerToggleEnd()
{
    if (mToggling)
    {
        outermost_context().timeline.mSelectedIntervals->endToggle();
        mToggling = false;
    }
}

}}} // namespace