#include "StateRightDown.h"

#include "UtilLog.h"
#include "StateIdle.h"
#include "Tooltip.h"
#include "Menu.h"
#include "Timeline.h"
#include "StateScrolling.h"
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

boost::statechart::result StateRightDown::react( const EvRightUp& evt )
{
    VAR_DEBUG(evt);
    mPopup = true;
    getMenuHandler().Popup();
    mPopup = false;
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
    if (mPopup) return discard_event();
    return transit<Idle>();
}

boost::statechart::result StateRightDown::react( const EvKeyDown& evt )
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