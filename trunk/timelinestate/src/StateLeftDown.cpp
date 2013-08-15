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

#include "StateLeftDown.h"

#include "Drag.h"
#include "EventDrag.h"
#include "EventKey.h"
#include "EventMouse.h"
#include "EventPart.h"
#include "Layout.h"
#include "Mouse.h"
#include "StateDragging.h"
#include "StateIdle.h"
#include "Tooltip.h"

#include "UtilLog.h"

namespace gui { namespace timeline { namespace state {

const wxString StateLeftDown::sTooltip = _(
    "Move the cursor to place the currently selected clips somewhere else."
    );

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

StateLeftDown::StateLeftDown( my_context ctx ) // entry
    :   TimeLineState( ctx )
    ,   mStartPosition(getMouse().getLeftDownPosition())
    ,   mSelectionEmpty(getSelection().isEmpty()) // Cached for performance
{
    LOG_DEBUG;
}

StateLeftDown::~StateLeftDown() // exit
{
    LOG_DEBUG;
}
//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result StateLeftDown::react( const EvLeftDown& evt )
{
    VAR_DEBUG(evt);
    FATAL("Not allowed");
    return discard_event();
}

boost::statechart::result StateLeftDown::react( const EvLeftUp& evt )
{
    VAR_DEBUG(evt);
    return transit<Idle>();
}

boost::statechart::result StateLeftDown::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);
    wxPoint diff = mStartPosition - getMouse().getVirtualPosition();
    if ((abs(diff.x) > Layout::DragThreshold) || (abs(diff.y) > Layout::DragThreshold))
    {
        if (!mSelectionEmpty)
        {
            // If no clip is selecting then starting a drag has no use.
            getDrag().start(getMouse().getVirtualPosition(), true);
            return transit<Dragging>();
        }
    }
    return forward_event();
}

boost::statechart::result StateLeftDown::react( const EvLeave& evt )
{
    VAR_DEBUG(evt);
    return transit<Idle>();
}

boost::statechart::result StateLeftDown::react( const EvKeyDown& evt)
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