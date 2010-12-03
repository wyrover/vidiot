#include "StateMoveDivider.h"

#include "StateIdle.h"
#include "UtilLog.h"
#include "Divider.h"

namespace gui { namespace timeline { namespace state {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

MoveDivider::MoveDivider( my_context ctx ) // entry
:   TimeLineState( ctx )
,   mOriginalPosition(getDivider().getPosition())
{
    LOG_DEBUG;
}

MoveDivider::~MoveDivider() // exit
{ 
    LOG_DEBUG; 
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result MoveDivider::react( const EvLeftDown& evt )
{
    VAR_DEBUG(evt);
    FATAL("Unexpected event.");
    return discard_event();
}

boost::statechart::result MoveDivider::react( const EvLeftUp& evt )
{
    VAR_DEBUG(evt);
    return transit<Idle>();
}

boost::statechart::result MoveDivider::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);
    getDivider().setPosition(evt.mPosition.y);
    return discard_event();
}

boost::statechart::result MoveDivider::react( const EvKeyDown& evt)
{
    switch (evt.mWxEvent.GetKeyCode())
    {
    case WXK_ESCAPE: return abort();
    }
    return discard_event();
}

boost::statechart::result MoveDivider::react( const EvKeyUp& evt)
{
    return discard_event();
}

boost::statechart::result MoveDivider::react( const EvLeave& evt)
{
    return abort();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result MoveDivider::abort()
{
    getDivider().setPosition(mOriginalPosition);
    return transit<Idle>();
}
}}} // namespace