#include "StateTestDragStart.h"

#include <boost/foreach.hpp>
#include "ClipView.h"
#include "Timeline.h"
#include "Drag.h"
#include "StateDragging.h"
#include "StateIdle.h"
#include "UtilLog.h"
#include "Selection.h"

namespace gui { namespace timeline { namespace state {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TestDragStart::TestDragStart( my_context ctx ) // entry
:   TimeLineState( ctx )
{
    LOG_DEBUG; 
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
    wxPoint diff = outermost_context().globals->DragStartPosition - evt.mPosition;
    static int tolerance = 2;
    if ((abs(diff.x) > tolerance) || (abs(diff.y) > tolerance))
    {
        getSelection().setDrag(true);

        getDrag().Start(evt.mPosition);

        //outermost_context().timeline.beginDrag(evt.mPosition);
        return transit<Dragging>();
    }
    return discard_event();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

}}} // namespace