#include "StateMoveDivider.h"

#include "StateIdle.h"
#include "UtilLog.h"
#include "Divider.h"
#include "Tooltip.h"

namespace gui { namespace timeline { namespace state {

    const wxString sTooltip = _("Move the cursor to 'scrub' over the timeline and see the frames back in the preview.");

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
    return forward_event();
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
    return forward_event();
}

boost::statechart::result MoveDivider::react( const EvKeyDown& evt)
{
    VAR_DEBUG(evt);
    switch (evt.mWxEvent.GetKeyCode())
    {
    case WXK_ESCAPE: 
        return abort();
    case WXK_F1:
        getTooltip().show(sTooltip);
        break;
    }
    return forward_event();
}

boost::statechart::result MoveDivider::react( const EvKeyUp& evt)
{
    VAR_DEBUG(evt);
    return forward_event();
}

boost::statechart::result MoveDivider::react( const EvLeave& evt)
{
    VAR_DEBUG(evt);
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