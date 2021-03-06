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

#include "StateTrim.h"

#include "CommandProcessor.h"
#include "EventDrag.h"
#include "EventKey.h"
#include "EventMouse.h"
#include "EventPart.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "StateIdle.h"
#include "Trim.h"

namespace gui { namespace timeline { namespace state {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

StateTrim::StateTrim( my_context ctx ) // entry
    : TimeLineState(ctx)
    , mShiftDown(getKeyboard().getShiftDown())
{
    LOG_DEBUG;
    getTrim().start();
    getSequenceView().setRealtimeRedrawing(true);
    model::CommandProcessor::get().enableUndoRedo(false);
}

StateTrim::~StateTrim() // exit
{
    LOG_DEBUG;
    model::CommandProcessor::get().enableUndoRedo(true);
    getSequenceView().setRealtimeRedrawing(false);
    getTrim().stop();
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

boost::statechart::result StateTrim::react( const EvLeftUp& evt )
{
    VAR_DEBUG(evt);
    getTrim().submit();
    PointerPositionInfo info = getMouse().getInfo(getMouse().getLeftDownPosition());
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

boost::statechart::result StateTrim::react( const EvMotion& evt )
{
    VAR_DEBUG(evt);
    getTrim().update();
    return forward_event();
}

boost::statechart::result StateTrim::react( const EvLeave& evt )
{
    VAR_DEBUG(evt);
    return transit<Idle>();
}

boost::statechart::result StateTrim::react( const EvKeyDown& evt)
{
    VAR_DEBUG(evt);
    switch (evt.KeyCode)
    {
    case WXK_ESCAPE:    
        evt.consumed();
        return transit<Idle>();
    case WXK_SHIFT:
        evt.consumed();
        if (!mShiftDown) // Avoid quirky feedback: when shift dragging, every motion event is followed by a key event. Updating on those key events causes flickering.
        {
            mShiftDown = true;
            getTrim().update();
        }
        break;
    case 'd':
    case 'D':           
        evt.consumed();
        getTrim().toggleSnapping();  
        break;
    case 'u':
    case 'U':           
        evt.consumed();
        getTrim().toggleTrimLink();
        getTrim().update();  
    }
    return forward_event();
}

boost::statechart::result StateTrim::react( const EvKeyUp& evt)
{
    VAR_DEBUG(evt);
    switch (evt.KeyCode)
    {
    case WXK_ESCAPE:    
        evt.consumed();
    case WXK_SHIFT:
        evt.consumed();
        if (mShiftDown) // Avoid quirky feedback: when shift dragging, every motion event is followed by a key event. Updating on those key events causes flickering.
        {
            mShiftDown = false;
            getTrim().update();
        }
        break;
    case 'd':
    case 'D':           
        evt.consumed();
        break;
    case 'u':
    case 'U':           
        evt.consumed();
        break;
    }
    return forward_event();
}

}}} // namespace