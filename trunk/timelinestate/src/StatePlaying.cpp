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

#include "StatePlaying.h"

#include "EventDrag.h"
#include "EventKey.h"
#include "EventMouse.h"
#include "EventPart.h"
#include "Intervals.h"
#include "Player.h"
#include "StateIdle.h"
#include "Tooltip.h"
#include "UtilLog.h"

namespace gui { namespace timeline { namespace state {

const wxString Playing::sTooltip = _(
    "Move the cursor to 'scrub' over the timeline and see the frames back in the preview.\n"
    );

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Playing::Playing( my_context ctx ) // entry
:   TimeLineState( ctx )
,   mMakingNewSelection(false)
{
    LOG_DEBUG;
}

Playing::~Playing() // exit
{
    LOG_DEBUG;
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result Playing::react( const EvLeftDown& evt )
{
    VAR_DEBUG(evt);
    post_event(evt); // Handle this event again in the Idle state
    getPlayer()->stop();
    triggerEnd();
    return transit< Idle >();
}

boost::statechart::result Playing::react( const EvRightDown& evt )
{
    VAR_DEBUG(evt);
    post_event(evt); // Handle this event again in the Idle state
    getPlayer()->stop();
    triggerEnd();
    return transit< Idle >();
}

boost::statechart::result Playing::react( const EvKeyDown& evt)
{
    VAR_DEBUG(evt);
    switch (evt.getKeyCode())
    {
    case WXK_SPACE:     getPlayer()->stop();        break;
    case WXK_SHIFT:     triggerBegin();             break;
    case WXK_F1:        getTooltip().show(sTooltip);break;
    }
    return forward_event();
}

boost::statechart::result Playing::react( const EvKeyUp& evt)
{
    VAR_DEBUG(evt);
    switch (evt.getKeyCode())
    {
    case WXK_SHIFT:     triggerEnd(); break;
    }
    return forward_event();
}

boost::statechart::result Playing::react( const EvPlaybackChanged& evt)
{
    VAR_DEBUG(evt);
    if (!evt.mActive)
    {
        triggerEnd();
        return transit< Idle >();
    }
    return discard_event();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Playing::triggerBegin()
{
    if (!mMakingNewSelection)
    {
        getIntervals().addBeginMarker();
        mMakingNewSelection = true;
    }
}

void Playing::triggerEnd()
{
    if (mMakingNewSelection)
    {
        getIntervals().addEndMarker();
        mMakingNewSelection = false;
    }
}

}}} // namespace