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
    if (evt.mWxEvent.ControlDown())
    {
        int nSteps = evt.mWxEvent.GetWheelRotation() / evt.mWxEvent.GetWheelDelta();
        VAR_INFO(nSteps);
        getZoom().change(nSteps);
    }
    return discard_event();
}

}}} // namespace

