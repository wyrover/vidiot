// Copyright 2013 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#include "StateScrolling.h"

#include "Drag.h"
#include "EventDrag.h"
#include "EventKey.h"
#include "EventMouse.h"
#include "EventPart.h"
#include "Menu.h"
#include "Mouse.h"
#include "PositionInfo.h"
#include "Scrolling.h"
#include "StateIdle.h"
#include "Timeline.h"
#include "Tooltip.h"
#include "UtilLog.h"
#include "Zoom.h"

namespace gui { namespace timeline { namespace state {

const wxString StateScrolling::sTooltip = _(
    "Drag mouse left/right to move the view\n"
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
    // This code is triggered in at least two scenarios:
    // - Right click a clip (popup menu is shown), then right click another clip.
    //   The second right click is immediately followed by a motion event causing the transition to this state.
    // - Sometimes a right down event is followed by a motion event (within that clip) due to slight mouse movement.
    //   Still, the user wants the popup.
    PointerPositionInfo infoDown = getMouse().getInfo(getMouse().getRightDownPosition());
    PointerPositionInfo infoUp = getMouse().getInfo(getMouse().getVirtualPosition());
    if (infoDown.clip == infoUp.clip)
    {
        getMenuHandler().Popup(getMouse().getRightDownPosition());
    }
    return transit<Idle>();
}

boost::statechart::result StateScrolling::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);
    getScrolling().align(getZoom().pixelsToPts(getMouse().getRightDownPosition().x), getMouse().getPhysicalPosition().x);
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

    switch (evt.getKeyCode())
    {
    case WXK_F1:        getTooltip().show(sTooltip); break;
    case WXK_ESCAPE:    return transit<Idle>();
    }
    return forward_event();
}

}}} // namespace