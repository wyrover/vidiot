#include "StateScrolling.h"

#include "Drag.h"
#include "EventDrag.h"
#include "EventKey.h"
#include "EventMouse.h"
#include "EventPart.h"
#include "Menu.h"
#include "MousePointer.h"
#include "Scrolling.h"
#include "StateIdle.h"
#include "Timeline.h"
#include "Tooltip.h"
#include "UtilLog.h"
#include "Zoom.h"

namespace gui { namespace timeline { namespace state {

    const wxString sTooltip = _(
        "Drag mouse left/right to move the view\n" \
        );

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

StateScrolling::StateScrolling( my_context ctx ) // entry
    :   TimeLineState( ctx )
{
    LOG_DEBUG;
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
    getScrolling().align(getZoom().pixelsToPts(getMousePointer().getRightDownPosition().x), evt.mWxEvent.GetPosition().x);
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