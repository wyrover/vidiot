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

#include "StateMoveDivider.h"

#include "CommandProcessor.h"
#include "EventDrag.h"
#include "EventKey.h"
#include "EventMouse.h"
#include "EventPart.h"
#include "Sequence.h"
#include "Mouse.h"
#include "SequenceView.h"
#include "StateIdle.h"

namespace gui { namespace timeline { namespace state {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

MoveDivider::MoveDivider( my_context ctx ) // entry
:   TimeLineState( ctx )
,   mOriginalPosition(getSequence()->getDividerPosition())
{
    LOG_DEBUG;
    model::CommandProcessor::get().enableUndoRedo(false);
}

MoveDivider::~MoveDivider() // exit
{
    LOG_DEBUG;
    model::CommandProcessor::get().enableUndoRedo(true);
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result MoveDivider::react( const EvLeftUp& evt )
{
    VAR_DEBUG(evt);
    return transit<Idle>();
}

boost::statechart::result MoveDivider::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);
    getSequenceView().setDividerPosition(mOriginalPosition + (getMouse().getVirtualPosition().y - getMouse().getLeftDownPosition().y));
    return forward_event();
}

boost::statechart::result MoveDivider::react( const EvLeave& evt)
{
    VAR_DEBUG(evt);
    return abort();
}

boost::statechart::result MoveDivider::react( const EvKeyDown& evt)
{
    VAR_DEBUG(evt);
    switch (evt.KeyCode)
    {
    case WXK_ESCAPE:    
        evt.consumed();
        return abort();
    }
    return forward_event();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result MoveDivider::abort()
{
    getSequenceView().setDividerPosition(mOriginalPosition);
    return transit<Idle>();
}
}}} // namespace