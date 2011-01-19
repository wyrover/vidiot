#include "StateAlways.h"

#include "Timeline.h"
#include "Zoom.h"
#include "UtilLog.h"

namespace gui { namespace timeline { namespace state {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Always::Always( my_context ctx ) // entry
:   TimeLineStateInner( ctx )
{
    LOG_DEBUG; 
}

Always::~Always() // exit
{ 
    LOG_DEBUG; 
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result Always::react( const EvWheel& evt )
{
    VAR_DEBUG(evt);
    int nSteps = evt.mWxEvent.GetWheelRotation() / evt.mWxEvent.GetWheelDelta();
    if (evt.mWxEvent.ControlDown())
    {
        getZoom().change(nSteps);
    }
    else if (evt.mWxEvent.ShiftDown())
    {
        int x;
        int y;
        getTimeline().GetViewStart(&x,&y);
        static const int sHorizontalScrollfactor = 25;
        getTimeline().Scroll(x - nSteps * sHorizontalScrollfactor,y);
    }
    else
    {
        // Only when this event is 'unhandled' by this state, then the original scrolling
        // behaviour should be done.
        evt.mWxEvent.Skip();
    }
    return discard_event();
}

}}} // namespace

