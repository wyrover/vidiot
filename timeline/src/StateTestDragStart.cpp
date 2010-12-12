#include "StateTestDragStart.h"

#include "Drag.h"
#include "StateDragging.h"
#include "StateIdle.h"
#include "UtilLog.h"

namespace gui { namespace timeline { namespace state {

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
        getDrag().Start(evt.mPosition);
        return transit<Dragging>();
    }
    return discard_event();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

}}} // namespace