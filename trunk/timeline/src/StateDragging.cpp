#include "StateDragging.h"

#include <boost/foreach.hpp>
#include "GuiTimeLineClip.h"
#include "Drag.h"
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

    getSelection().setDrag(false);

    getDrag().Stop();

    return transit<Idle>();
}

boost::statechart::result Dragging::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);

    getDrag().MoveTo(evt.mPosition);
    getDrop().updateDropArea(evt.mPosition);

    return discard_event();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Dragging::showDropArea(wxPoint p)
{
}

}}} // namespace