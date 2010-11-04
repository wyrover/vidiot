#include "StateTestDragStart.h"

#include <boost/foreach.hpp>
#include "GuiTimeLineClip.h"
#include "GuiTimeLine.h"
#include "StateDragging.h"
#include "StateIdle.h"
#include "UtilLog.h"

namespace gui { namespace timeline { namespace state {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TestDragStart::TestDragStart( my_context ctx ) // entry
:   my_base( ctx )
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
        outermost_context().globals->selection.setDrag(true);

        // Begin the drag operation
        GuiTimeLine& timeline = outermost_context().timeline;
        outermost_context().globals->DragImage = new GuiTimeLineDragImage(&timeline, evt.mPosition);
        GuiTimeLineDragImage* dragimage = outermost_context().globals->DragImage;
        bool ok = dragimage->BeginDrag(dragimage->getHotspot(), &timeline, false);
        ASSERT(ok);
        timeline.Refresh(false);
        timeline.Update();
        dragimage->Move(evt.mPosition);
        dragimage->Show();

        //outermost_context().timeline.beginDrag(evt.mPosition);
        return transit<Dragging>();
    }
    return discard_event();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

}}} // namespace