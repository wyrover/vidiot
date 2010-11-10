#include "StateDragging.h"

#include <boost/foreach.hpp>
#include "GuiTimeLineClip.h"
#include "GuiTimeLineDragImage.h"
#include "GuiTimeLine.h"
#include "GuiTimeLineTrack.h"
#include "StateIdle.h"
#include "UtilLog.h"
#include "ViewMap.h"

namespace gui { namespace timeline { namespace state {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

/** /todo handle mouse focus lost */

Dragging::Dragging( my_context ctx ) // entry
:   TimeLineState( ctx )
//,   mClip()
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

    getSelectClips().setDrag(false);

    // End the drag operation
    GuiTimeLineDragImage* dragimage = getTimeline().getDragImage();
    dragimage->Hide();
    dragimage->EndDrag();
    getTimeline().Refresh();
    getTimeline().setDragImage(0);
    delete dragimage;

    return transit<Idle>();
}

boost::statechart::result Dragging::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);

    // Move the drag image
    GuiTimeLineDragImage* dragimage = getTimeline().getDragImage();
    //dragimage->Hide();
    getTimeline().Refresh(false);
    getTimeline().Update();
    //dragimage->Show();
    dragimage->Move(evt.mPosition - getTimeline().getScrollOffset());

    showDropArea(evt.mPosition); 

    return discard_event();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Dragging::showDropArea(wxPoint p)
{
    GuiTimeLineDragImage* dragimage = getTimeline().getDragImage();
    PointerPositionInfo info = getTimeline().getPointerInfo(p);

    if (info.track)
    {
        GuiTimeLineTrack* track = getViewMap().getView(info.track);
        if (info.clip)
        {
            GuiTimeLineClip* clip = getViewMap().getView(info.clip);
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
            getTimeline().showDropArea(wxRect(xDrop,info.trackPosition,4,track->getBitmap().GetHeight())); 
        }
        else
        {
            getTimeline().showDropArea(wxRect(p.x,info.trackPosition,4,track->getBitmap().GetHeight())); 
        }
    }
    else
    {
        getTimeline().showDropArea(wxRect(0,0,0,0));
    }
}

}}} // namespace