// Copyright 2013-2015 Eric Raijmakers.
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

#include "StateMovingCursor.h"

#include "Cursor.h"
#include "EventDrag.h"
#include "EventKey.h"
#include "EventMouse.h"
#include "EventPart.h"
#include "Keyboard.h"
#include "Intervals.h"
#include "Mouse.h"
#include "StateIdle.h"
#include "Tooltip.h"
#include "UtilLog.h"

namespace gui { namespace timeline { namespace state {

const wxString MovingCursor::sTooltip = _(
    "Move the cursor to 'scrub' over the timeline and see the frames back in the preview.\n"
    );

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

MovingCursor::MovingCursor( my_context ctx ) // entry
:   TimeLineState( ctx )
,   mToggling(false)
{
    LOG_DEBUG;

    getCursor().setLogicalPosition(getZoom().pixelsToPts(getMouse().getVirtualPosition().x));
    if (getKeyboard().getShiftDown())
    {
        triggerToggleStart();
    }
}

MovingCursor::~MovingCursor() // exit
{
    LOG_DEBUG;
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result MovingCursor::react( const EvLeftDown& evt )
{
    VAR_DEBUG(evt);
    getCursor().setLogicalPosition(getZoom().pixelsToPts(getMouse().getVirtualPosition().x));
    if (getKeyboard().getShiftDown())
    {
        triggerToggleStart();
    }
    return forward_event();
}

boost::statechart::result MovingCursor::react( const EvLeftUp& evt )
{
    VAR_DEBUG(evt);
    if (getKeyboard().getShiftDown())
    {
        triggerToggleEnd();
    }
    return transit<Idle>();
}

boost::statechart::result MovingCursor::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);
    getCursor().setLogicalPosition(getZoom().pixelsToPts(getMouse().getVirtualPosition().x));
    return forward_event();
}

boost::statechart::result MovingCursor::react( const EvLeave& evt)
{
    VAR_DEBUG(evt);
    if (getKeyboard().getShiftDown())
    {
        triggerToggleEnd();
    }
    return transit<Idle>();
}

boost::statechart::result MovingCursor::react( const EvKeyDown& evt)
{
    VAR_DEBUG(evt);
    switch (evt.KeyCode)
    {
    case WXK_SHIFT:     triggerToggleStart();        break;
    case WXK_F1:        getTooltip().show(sTooltip); break;
    }
    return forward_event();
}

boost::statechart::result MovingCursor::react( const EvKeyUp& evt)
{
    VAR_DEBUG(evt);
    switch (evt.KeyCode)
    {
    case WXK_SHIFT:     
        evt.consumed();
        triggerToggleEnd(); 
        break;
    }
    return forward_event();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void MovingCursor::triggerToggleStart()
{
    if (!mToggling)
    {
        getIntervals().startToggle();
        mToggling = true;
    }
}

void MovingCursor::triggerToggleEnd()
{
    if (mToggling)
    {
        getIntervals().endToggle();
        mToggling = false;
    }
}

}}} // namespace