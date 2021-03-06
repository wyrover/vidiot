// Copyright 2013-2016 Eric Raijmakers.
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

#include "StateRightDown.h"

#include "CommandProcessor.h"
#include "EventDrag.h"
#include "EventKey.h"
#include "EventMouse.h"
#include "EventPart.h"
#include "Menu.h"
#include "Mouse.h"
#include "StateIdle.h"
#include "StateScrolling.h"
#include "Timeline.h"
#include "Zoom.h"

namespace gui { namespace timeline { namespace state {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

StateRightDown::StateRightDown( my_context ctx ) // entry
    :   TimeLineState( ctx )
{
    LOG_DEBUG;
    model::CommandProcessor::get().enableUndoRedo(false);
}

StateRightDown::~StateRightDown() // exit
{
    LOG_DEBUG;
    model::CommandProcessor::get().enableUndoRedo(true);
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
    getMenuHandler().popup();
    return transit<Idle>();
}
boost::statechart::result StateRightDown::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);
    // State scrolling is mostly used to avoid showing the popup menu.
    // Due to the scrolling, the popup menu might apply to a clip that is
    // not in the current visible region, leading to unexpected behaviour.
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

    switch (evt.KeyCode)
    {
    case WXK_ESCAPE:    
        evt.consumed();
        return transit<Idle>();
    }
    return forward_event();
}

}}} // namespace