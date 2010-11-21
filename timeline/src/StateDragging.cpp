#include "StateDragging.h"

#include <boost/foreach.hpp>
#include "ClipView.h"
#include "Drag.h"
#include "Timeline.h"
#include "TrackView.h"
#include "StateIdle.h"
#include "UtilLog.h"
#include "ViewMap.h"
#include "Drop.h"
#include "Selection.h"

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

    //getSelection().setDrag(false);

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