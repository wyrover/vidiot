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

    BOOST_FOREACH( GuiTimeLineClipPtr c, outermost_context().timeline.getClips() )
    {
        c->setBeingDragged(false);
    }

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
    boost::tuple<GuiTimeLineTrackPtr,int> tt = timeline.findTrack(p.y);
    boost::tuple<GuiTimeLineClipPtr,int> cw = timeline.findClip(p);
    GuiTimeLineTrackPtr track = tt.get<0>();
    GuiTimeLineClipPtr clip = cw.get<0>();
    mClip = clip;

    if (track)
    {
        if (clip)
        {
            boost::tuple<int,int> clipbounds = track->findClipBounds(clip);

            int diffleft  = p.x - clipbounds.get<0>();
            int diffright = clipbounds.get<1>() - p.x;

            int xDrop = -1;
            if (diffleft < diffright)
            {
                xDrop = clipbounds.get<0>() - 2;
            }
            else
            {
                xDrop = clipbounds.get<1>() - 2;
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