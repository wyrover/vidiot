#include "StateDragging.h"

#include <boost/foreach.hpp>
#include "GuiTimeLineClip.h"
#include "GuiTimeLine.h"
#include "GuiTimeLineTrack.h"
#include "StateIdle.h"
#include "UtilLog.h"

namespace gui { namespace timeline { namespace state {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

/** /todo handle mouse focus lost */

Dragging::Dragging( my_context ctx ) // entry
:   my_base( ctx )
,   mClip()
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

    outermost_context().globals->selection.setDrag(false);

    // End the drag operation
    GuiTimeLine& timeline = outermost_context().timeline;
    GuiTimeLineDragImage* dragimage = outermost_context().globals->DragImage;
    dragimage->Hide();
    dragimage->EndDrag();
    timeline.Refresh();
    delete outermost_context().globals->DragImage;
    outermost_context().globals->DragImage = 0;

    return transit<Idle>();
}

boost::statechart::result Dragging::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);

    // Move the drag image
    GuiTimeLine& timeline = outermost_context().timeline;
    GuiTimeLineDragImage* dragimage = outermost_context().globals->DragImage;
    //dragimage->Hide();
    timeline.Refresh(false);
    timeline.Update();
    //dragimage->Show();
    dragimage->Move(evt.mPosition - timeline.getScrollOffset());

    showDropArea(evt.mPosition); 

    return discard_event();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Dragging::showDropArea(wxPoint p)
{
    GuiTimeLine& timeline = outermost_context().timeline;
    GuiTimeLineDragImage* dragimage = outermost_context().globals->DragImage;
    boost::tuple<model::TrackPtr,int> tt =  (timeline.findTrack(p.y));
    GuiTimeLineTrackPtr track = outermost_context().globals->mViewMap.ModelToView(tt.get<0>());
    GuiTimeLineClipPtr clip = timeline.findClip(p);
    mClip = clip;

    if (track)
    {
        if (clip)
        {
            int diffleft  = p.x - clip->getLeftPosition();
            int diffright = clip->getRightPosition() - p.x;

            int xDrop = -1;
            if (diffleft < diffright)
            {
                xDrop = clip->getLeftPosition() - 2;
            }
            else
            {
                xDrop = clip->getRightPosition() - 2;
            }
            timeline.showDropArea(wxRect(xDrop,tt.get<1>(),4,track->getBitmap().GetHeight())); 
        }
        else
        {
            timeline.showDropArea(wxRect(p.x,tt.get<1>(),4,track->getBitmap().GetHeight())); 
        }
    }
    else
    {
        timeline.showDropArea(wxRect(0,0,0,0));
    }
}

}}} // namespace