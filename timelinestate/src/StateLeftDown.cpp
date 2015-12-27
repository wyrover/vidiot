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

#include "StateLeftDown.h"

#include "Drag.h"
#include "EventDrag.h"
#include "EventKey.h"
#include "EventMouse.h"
#include "EventPart.h"
#include "Mouse.h"
#include "StateDragging.h"
#include "StateIdle.h"

namespace gui { namespace timeline { namespace state {

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
    PointerPositionInfo info{ getMouse().getInfo(getMouse().getLeftDownPosition()) };
    if (info.onAudioVideoDivider)
    {
        // Nothing
    }
    else if (info.onTrackDivider)
    {
        // Nothing
    }
    else
    {
        getSelection().updateOnLeftUp(info);
    }

    return transit<Idle>();
}

boost::statechart::result StateLeftDown::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);
    wxPoint diff = mStartPosition - getMouse().getVirtualPosition();
    if ((abs(diff.x) > Drag::Threshold) || (abs(diff.y) > Drag::Threshold))
    {
        if (!mSelectionEmpty)
        {
            // If no clip is selected then starting a drag has no use.

            PointerPositionInfo info = getMouse().getInfo(mStartPosition);
            if (info.clip != nullptr &&
                info.track != nullptr)
            {
                // Only if the mouse button was down on a track when pressing the button,
                // the drag may be started. This check is required to ensure that the Drag
                // code is able to determine the starting clip and track of the drag.
                getDrag().start(mStartPosition, false);
                return transit<Dragging>();
            }

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
    switch (evt.KeyCode)
    {
    case WXK_ESCAPE:    
        evt.consumed();
        return transit<Idle>();
    }
    return forward_event();
}

}}} // namespace