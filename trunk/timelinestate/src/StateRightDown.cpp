#include "StateRightDown.h"

#include "EventDrag.h"
#include "EventKey.h"
#include "EventMouse.h"
#include "EventPart.h"
#include "Menu.h"
#include "MousePointer.h"
#include "StateIdle.h"
#include "StateScrolling.h"
#include "Timeline.h"
#include "Tooltip.h"
#include "UtilLog.h"
#include "Zoom.h"

namespace gui { namespace timeline { namespace state {

    const wxString sTooltip = _(
        "\n" \
        );

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

StateRightDown::StateRightDown( my_context ctx ) // entry
    :   TimeLineState( ctx )
{
    LOG_DEBUG;
}

StateRightDown::~StateRightDown() // exit
{
    LOG_DEBUG;
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result StateRightDown::react( const EvRightDown& evt )
{
    VAR_DEBUG(evt);
    return transit<Idle>();
}

boost::statechart::result StateRightDown::react( const EvRightUp& evt )
{
    VAR_DEBUG(evt);
    getMenuHandler().Popup(getMousePointer().getRightDownPosition());
    return transit<Idle>();
}
boost::statechart::result StateRightDown::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);
    return transit<StateScrolling>();
}

boost::statechart::result StateRightDown::react( const EvLeave& evt )
{
    VAR_DEBUG(evt);
    return transit<Idle>();
}

boost::statechart::result StateRightDown::react( const EvKeyDown& evt )
{
    VAR_DEBUG(evt);

    switch (evt.getKeyCode())
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