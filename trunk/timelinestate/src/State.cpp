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

#include "State.h"

#include "EventDrag.h"
#include "EventKey.h"
#include "EventMouse.h"
#include "EventPart.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Player.h"
#include "Scrolling.h"
#include "StateIdle.h"
#include "Timeline.h"
#include "UtilException.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "VideoDisplayEvent.h"
#include "Zoom.h"

namespace gui { namespace timeline { namespace state {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Machine::Machine(Timeline& tl)
:   Part(&tl)
{
    initiate();

    getTimeline().  Bind(wxEVT_MOTION,                  &Machine::onMotion,         this);
    getTimeline().  Bind(wxEVT_LEFT_DOWN,               &Machine::onLeftDown,       this);
    getTimeline().  Bind(wxEVT_LEFT_UP,                 &Machine::onLeftUp,         this);
    getTimeline().  Bind(wxEVT_LEFT_DCLICK,             &Machine::onLeftDouble,     this);
    getTimeline().  Bind(wxEVT_MIDDLE_DOWN,             &Machine::onMiddleDown,     this);
    getTimeline().  Bind(wxEVT_MIDDLE_UP,               &Machine::onMiddleUp,       this);
    getTimeline().  Bind(wxEVT_MIDDLE_DCLICK,           &Machine::onMiddleDouble,   this);
    getTimeline().  Bind(wxEVT_RIGHT_DOWN,              &Machine::onRightDown,      this);
    getTimeline().  Bind(wxEVT_RIGHT_UP,                &Machine::onRightUp,        this);
    getTimeline().  Bind(wxEVT_RIGHT_DCLICK,            &Machine::onRightDouble,    this);
    getTimeline().  Bind(wxEVT_ENTER_WINDOW,            &Machine::onEnter,          this);
    getTimeline().  Bind(wxEVT_LEAVE_WINDOW,            &Machine::onLeave,          this);
    getTimeline().  Bind(wxEVT_MOUSEWHEEL,              &Machine::onWheel,          this);
    getTimeline().  Bind(wxEVT_KEY_DOWN,                &Machine::onKeyDown,        this);
    getTimeline().  Bind(wxEVT_KEY_UP,                  &Machine::onKeyUp,          this);
    getTimeline().  Bind(wxEVT_MOUSE_CAPTURE_LOST,      &Machine::onCaptureLost,    this);
    getTimeline().  Bind(wxEVT_MOUSE_CAPTURE_CHANGED,   &Machine::onCaptureChanged, this);

    getPlayer()->Bind(EVENT_PLAYBACK_ACTIVE, &Machine::onPlaybackActive, this);
    getPlayer()->Bind(EVENT_PLAYBACK_POSITION, &Machine::onPlaybackPosition, this);

    VAR_DEBUG(this);
}

Machine::~Machine()
{
    VAR_DEBUG(this);

    getTimeline().  Unbind(wxEVT_MOTION,                &Machine::onMotion,         this);
    getTimeline().  Unbind(wxEVT_LEFT_DOWN,             &Machine::onLeftDown,       this);
    getTimeline().  Unbind(wxEVT_LEFT_UP,               &Machine::onLeftUp,         this);
    getTimeline().  Unbind(wxEVT_LEFT_DCLICK,           &Machine::onLeftDouble,     this);
    getTimeline().  Unbind(wxEVT_MIDDLE_DOWN,           &Machine::onMiddleDown,     this);
    getTimeline().  Unbind(wxEVT_MIDDLE_UP,             &Machine::onMiddleUp,       this);
    getTimeline().  Unbind(wxEVT_MIDDLE_DCLICK,         &Machine::onMiddleDouble,   this);
    getTimeline().  Unbind(wxEVT_RIGHT_DOWN,            &Machine::onRightDown,      this);
    getTimeline().  Unbind(wxEVT_RIGHT_UP,              &Machine::onRightUp,        this);
    getTimeline().  Unbind(wxEVT_RIGHT_DCLICK,          &Machine::onRightDouble,    this);
    getTimeline().  Unbind(wxEVT_ENTER_WINDOW,          &Machine::onEnter,          this);
    getTimeline().  Unbind(wxEVT_LEAVE_WINDOW,          &Machine::onLeave,          this);
    getTimeline().  Unbind(wxEVT_MOUSEWHEEL,            &Machine::onWheel,          this);
    getTimeline().  Unbind(wxEVT_KEY_DOWN,              &Machine::onKeyDown,        this);
    getTimeline().  Unbind(wxEVT_KEY_UP,                &Machine::onKeyUp,          this);
    getTimeline().  Unbind(wxEVT_MOUSE_CAPTURE_LOST,    &Machine::onCaptureLost,    this);
    getTimeline().  Unbind(wxEVT_MOUSE_CAPTURE_CHANGED, &Machine::onCaptureChanged, this);

    getPlayer()->Unbind(EVENT_PLAYBACK_ACTIVE, &Machine::onPlaybackActive, this);
    getPlayer()->Unbind(EVENT_PLAYBACK_POSITION, &Machine::onPlaybackPosition, this);

}

struct EvStart : boost::statechart::event< EvStart > {};

struct Starting
    :   public boost::statechart::simple_state<Starting, Machine>
{
    typedef boost::statechart::transition< EvStart, Idle > reactions;
};

void Machine::start()
{
    process_event(EvStart());
}

//////////////////////////////////////////////////////////////////////////
// BOOST STATECHART OVERRIDES
//////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
    static int __state_prefix_length = std::string("struct gui::timeline::state::").size();
#else
    static int __state_prefix_length = std::string("gui::timeline::state::").size();
#endif

void Machine::unconsumed_event( const boost::statechart::event_base & evt )
{
    LOG_DEBUG << "[state="  <<  boost::units::detail::demangle(typeid( *state_begin() ).name()).substr(__state_prefix_length)
              << "][event=" <<  boost::units::detail::demangle(typeid( evt ).name()).substr(__state_prefix_length)
              << "]";
    boost::statechart::state_machine< Machine, Starting >::unconsumed_event(evt);
}

void  Machine::process_event(const boost::statechart::event_base & evt )
{
    boost::statechart::state_machine< Machine, Starting >::process_event(evt);
    LOG_DEBUG <<  "[event="    <<  boost::units::detail::demangle(typeid( evt ).name()).substr(__state_prefix_length)
              << "][newstate=" << (state_begin() == state_end() ? std::string("???") :  boost::units::detail::demangle(typeid( *state_begin() ).name()).substr(__state_prefix_length))
              << "]";
}

//////////////////////////////////////////////////////////////////////////
// HANDLING EVENTS - PUBLIC FOR REUSE IN TEST CODE
//////////////////////////////////////////////////////////////////////////

bool Machine::processWheelEvent(int nSteps)
{
    VAR_INFO(nSteps);
    bool processed = false;
    // Zooming/Scrolling can be done in any state
    if (getKeyboard().getCtrlDown())
    {
        getZoom().change(nSteps);
        processed = true;
    }
    else if (getKeyboard().getShiftDown())
    {
        int x;
        int y;
        getTimeline().GetViewStart(&x,&y);
        static const int sHorizontalScrollfactor = 100;
        getTimeline().Scroll(x - nSteps * sHorizontalScrollfactor,y);
        processed = true;
    }
    return processed;
}

void Machine::onMotion(wxMouseEvent& event)
{
    CatchExceptions([this, &event]
    {
        EvMotion motionEvent(event);
        handleMotion(motionEvent);
        event.Skip();
    });
}

void Machine::handleMotion(EvMotion& event)
{
    VAR_DEBUG(event);
    getMouse().update(event);
    getScrolling().update(event); // For right-mouse scrolling
    process_event(event);
}

void Machine::onLeftDown(wxMouseEvent& event)
{
    CatchExceptions([this, &event]
    {
        EvLeftDown leftDownEvent(event);
        handleLeftDown(leftDownEvent);
        event.Skip();
    });
}

void Machine::handleLeftDown(EvLeftDown& event)
{
    VAR_DEBUG(event);
    getMouse().update(event);
    getMouse().setLeftDown(true);
    getTimeline().SetFocus();
    process_event(event);
}

void Machine::onLeftUp(wxMouseEvent& event)
{
    CatchExceptions([this, &event]
    {
        EvLeftUp leftUpEvent(event);
        handleLeftUp(leftUpEvent);
        event.Skip();
    });
}

void Machine::handleLeftUp(EvLeftUp& event)
{
    VAR_DEBUG(event);
    getMouse().update(event);
    getMouse().setLeftDown(false);
    process_event(event);
}

void Machine::onLeftDouble(wxMouseEvent& event)
{
    CatchExceptions([this, &event]
    {
        EvLeftDouble eventLeftDouble(event);
        handleLeftDouble(eventLeftDouble);
        event.Skip();
    });
}

void Machine::handleLeftDouble(EvLeftDouble& event)
{
    VAR_DEBUG(event);
    getMouse().update(event);
    getMouse().setLeftDown(true);
    process_event(event);
}

void Machine::onMiddleDown(wxMouseEvent& event)
{
    CatchExceptions([this, &event]
    {
        EvMiddleDown eventMiddleDown(event);
        handleMiddleDown(eventMiddleDown);
        event.Skip();
    });
}

void Machine::handleMiddleDown(EvMiddleDown& event)
{
    VAR_DEBUG(event);
    getMouse().update(event);
    process_event(event);
}

void Machine::onMiddleUp(wxMouseEvent& event)
{
    CatchExceptions([this, &event]
    {
        EvMiddleUp eventMiddleUp(event);
        handleMiddleUp(eventMiddleUp);
        event.Skip();
    });
}

void Machine::handleMiddleUp(EvMiddleUp& event)
{
    VAR_DEBUG(event);
    getMouse().update(event);
    process_event(event);
}

void Machine::onMiddleDouble(wxMouseEvent& event)
{
    CatchExceptions([this, &event]
    {
        EvMiddleDouble eventMiddleDouble(event);
        handleMiddleDouble(eventMiddleDouble);
        event.Skip();
    });
}

void Machine::handleMiddleDouble(EvMiddleDouble& event)
{
    VAR_DEBUG(event);
    getMouse().update(event);
    process_event(event);
}

void Machine::onRightDown(wxMouseEvent& event)
{
    CatchExceptions([this, &event]
    {
        EvRightDown eventRightDown(event);
        handleRightDown(eventRightDown);
        event.Skip();
    });
}

void Machine::handleRightDown(EvRightDown& event)
{
    VAR_DEBUG(event);
    getMouse().update(event);
    getMouse().setRightDown(true);
    getScrolling().rightDown(); // For right-mouse scrolling
    getTimeline().SetFocus();
    process_event(event);
}

void Machine::onRightUp(wxMouseEvent& event)
{
    CatchExceptions([this, &event]
    {
        EvRightUp eventRightUp(event);
        handleRightUp(eventRightUp);
        event.Skip();
    });
}

void Machine::handleRightUp(EvRightUp& event)
{
    VAR_DEBUG(event);
    getMouse().setRightDown(false);
    getMouse().update(event);
    process_event(event);
}

void Machine::onRightDouble(wxMouseEvent& event)
{
    CatchExceptions([this, &event]
    {
        EvRightDouble eventRightDouble(event);
        handleRightDouble(eventRightDouble);
        event.Skip();
    });
}

void Machine::handleRightDouble(EvRightDouble& event)
{
    VAR_DEBUG(event);
    getMouse().update(event);
    getMouse().setRightDown(true);
    process_event(event);
 }

void Machine::onEnter(wxMouseEvent& event)
{
    CatchExceptions([this, &event]
    {
        EvEnter eventEnter(event);
        handleEnter(eventEnter);
        event.Skip();
    });
}

void Machine::handleEnter(EvEnter& event)
{
    VAR_DEBUG(event);
    getMouse().update(event);
    getScrolling().rightDown(); // Avoid 'flipping' (due to a wrongly initialized right mouse down position) when entering the widget with the right mouse button pressed.
    process_event(event);
}

void Machine::onLeave(wxMouseEvent& event)
{
    CatchExceptions([this, &event]
    {
        EvLeave eventLeave(event);
        handleLeave(eventLeave);
        event.Skip();
    });
}

void Machine::handleLeave(EvLeave& event)
{
    VAR_DEBUG(event);
    getMouse().update(event);
    getMouse().setLeftDown(false);
    getMouse().setRightDown(false);
    process_event(event);
}

void Machine::onWheel(wxMouseEvent& event)
{
    CatchExceptions([this, &event]
    {
        VAR_DEBUG(event);

        // Zooming/Scrolling can be done in any state
        int nSteps = event.GetWheelRotation() / event.GetWheelDelta();
        if (!processWheelEvent(nSteps))
        {
            // Only when this event is 'unhandled' here, the original scrolling
            // behaviour should be done.
            event.Skip();
        }
        // NOT: process_event(EvWheel()); -- Unused
    });
}

void Machine::onKeyDown(wxKeyEvent& event)
{
    CatchExceptions([this, &event]
    {
        // By default, the event may propagate upwards.
        // If handled by the state machine, Skip(false) will be called.
        // For instance, Don't want the left/down keys to propagate further,
        // since that causes scrolling by wxScrolledWindow
        event.Skip();
        EvKeyDown eventKeyDown(event);
        handleKeyDown(eventKeyDown);
    });
}

void Machine::handleKeyDown(EvKeyDown& event)
{
    VAR_DEBUG(event);
    getKeyboard().update(event);

    process_event(EvKeyDown(event));
}

void Machine::onKeyUp(wxKeyEvent& event)
{
    CatchExceptions([this, &event]
    {
        // By default, the event may propagate upwards.
        // If handled by the state machine, Skip(false) will be called.
        // For instance, Don't want the left/down keys to propagate further,
        // since that causes scrolling by wxScrolledWindow
        event.Skip();
        EvKeyUp eventKeyUp(event);
        handleKeyUp(eventKeyUp);
    });
}

void Machine::handleKeyUp(EvKeyUp& event)
{
    VAR_DEBUG(event);
    getKeyboard().update(event);
    process_event(EvKeyUp(event));
}

void Machine::onCaptureLost(wxMouseCaptureLostEvent& event)
{
    LOG_DEBUG;
};

void Machine::onCaptureChanged(wxMouseCaptureChangedEvent& event)
{
    LOG_DEBUG;
};

void Machine::onPlaybackActive(PlaybackActiveEvent& event)
{
    LOG_DEBUG;
    process_event(EvPlaybackChanged(event.getValue()));
    event.Skip();
}

void Machine::onPlaybackPosition(PlaybackPositionEvent& event)
{
    LOG_DEBUG;
    process_event(EvPlaybackPositionChanged(event.getValue()));
    event.Skip();
}

}}} // namespace
