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

#include "StateDragging.h"

#include "Drag.h"
#include "EventDrag.h"
#include "EventKey.h"
#include "EventMouse.h"
#include "EventPart.h"
#include "Mouse.h"
#include "StateIdle.h"
#include "Timeline.h"
#include "UtilLog.h"

namespace gui { namespace timeline { namespace state {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Dragging::Dragging( my_context ctx ) // entry
    :   TimeLineState( ctx )
{
    LOG_DEBUG;

}

Dragging::~Dragging() // exit
{
    LOG_DEBUG;
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result Dragging::react( const EvLeftUp& evt )
{
    // See also EvDragDrop
    VAR_DEBUG(evt);
    getDrag().drop();
    getDrag().stop();
    return transit<Idle>();
}

boost::statechart::result Dragging::react( const EvDragDrop& evt )
{
    // See also EvLeftUp
    // No mouse events are generated during drag&drop, only these Drag&Drop events.
    VAR_DEBUG(evt);
    getDrag().drop();
    getDrag().stop();
    return transit<Idle>();
}

boost::statechart::result Dragging::react( const EvMotion& evt )
{
    // See also EvDragMove
    VAR_DEBUG(evt);
    getDrag().move(getMouse().getVirtualPosition());
    return forward_event();
}

boost::statechart::result Dragging::react( const EvDragMove& evt )
{
    // See also EvMotion
    // No mouse events are generated during drag&drop, only these Drag&Drop events.
    VAR_DEBUG(evt);
    getDrag().move(getMouse().getVirtualPosition());
    return forward_event();
}

boost::statechart::result Dragging::react( const EvLeave& evt )
{
    // See also EvDragEnd
    VAR_DEBUG(evt);
    getDrag().stop();
    return transit<Idle>();
}

boost::statechart::result Dragging::react( const EvDragEnd& evt )
{
    // See also EvLeave
    // No mouse events are generated during drag&drop, only these Drag&Drop events.
    VAR_DEBUG(evt);
    getDrag().stop();
    return transit<Idle>();
}

boost::statechart::result Dragging::react( const EvKeyDown& evt )
{
    VAR_DEBUG(evt);

    switch (evt.KeyCode)
    {
    case WXK_CONTROL:
    case WXK_SHIFT:     
        evt.consumed();
        getDrag().move(getMouse().getVirtualPosition()); 
        break;
    case WXK_ESCAPE:    
        evt.consumed();
        getDrag().stop(); return transit<Idle>();        
        break;
    case 'd':
    case 'D':           
        evt.consumed();
        getDrag().toggleSnapping();                      
        break;
    }
    return forward_event();
}

boost::statechart::result Dragging::react( const EvKeyUp& evt )
{
    VAR_DEBUG(evt);

    switch (evt.KeyCode)
    {
    case WXK_CONTROL:
    case WXK_SHIFT:     
        evt.consumed();
        getDrag().move(getMouse().getVirtualPosition()); 
        break;
    }
    return forward_event();
}

}}} // namespace