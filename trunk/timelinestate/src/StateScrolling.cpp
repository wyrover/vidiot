#include "StateScrolling.h"

#include "Drag.h"
#include "UtilLog.h"
#include "StateIdle.h"
#include "Tooltip.h"
#include "Timeline.h"
#include "Scrolling.h"
#include "Zoom.h"

namespace gui { namespace timeline { namespace state {

    const wxString sTooltip = _(
        "\n" \
        );

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

/** /todo handle mouse focus lost */

StateScrolling::StateScrolling( my_context ctx ) // entry
    :   TimeLineState( ctx )
{
    LOG_DEBUG; 

    const EvRightDown* event = dynamic_cast<const EvRightDown*>(triggering_event());
    ASSERT(event); // Only way to get here is to press left button in the Idle state

    mPosition = getZoom().pixelsToPts(event->mPosition.x);
    // We need the position when the mouse button is clicked, not the position of the first motion.
    // Hence, there is no 'test start scrolling' state.
}

StateScrolling::~StateScrolling() // exit
{ 
    LOG_DEBUG; 
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result StateScrolling::react( const EvRightUp& evt )
{
    VAR_DEBUG(evt);
    return transit<Idle>();
}

boost::statechart::result StateScrolling::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);
    getScrolling().align(mPosition, getZoom().pixelsToPts(evt.mPosition.x));
    return forward_event();
}

boost::statechart::result StateScrolling::react( const EvLeave& evt )
{
    VAR_DEBUG(evt);
    return transit<Idle>();
}

boost::statechart::result StateScrolling::react( const EvKeyDown& evt )
{
    VAR_DEBUG(evt);

    switch (evt.mWxEvent.GetKeyCode())
    {
    case WXK_F1:
        getTooltip().show(sTooltip);
        break;
    case WXK_ESCAPE:
        return transit<Idle>();
    }
    return forward_event();
}

}}} // namespace