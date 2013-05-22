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
#include "UtilLog.h"
#include "VideoDisplayEvent.h"
#include "Zoom.h"

namespace gui { namespace timeline { namespace state {

//////////////////////////////////////////////////////////////////////////
// MACHINE
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
    getZoom().      Bind(ZOOM_CHANGE_EVENT,             &Machine::onZoomChanged,    this);
    getScrolling(). Bind(SCROLL_CHANGE_EVENT,           &Machine::onScrollChanged,  this);

    getPlayer()->Bind(EVENT_PLAYBACK_ACTIVE, &Machine::onPlaybackActive, this);

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
    getZoom().      Unbind(ZOOM_CHANGE_EVENT,           &Machine::onZoomChanged,    this);
    getScrolling(). Unbind(SCROLL_CHANGE_EVENT,         &Machine::onScrollChanged,  this);

    getPlayer()->Unbind(EVENT_PLAYBACK_ACTIVE, &Machine::onPlaybackActive, this);
}

void Machine::unconsumed_event( const boost::statechart::event_base & evt )
{
    LOG_DEBUG << "[state=" << typeid( *state_begin() ).name() << "][event=" << typeid( evt ).name() << "]";
    boost::statechart::state_machine< Machine, Idle >::unconsumed_event(evt);
}

void  Machine::process_event(const boost::statechart::event_base & evt )
{
    static int pos = std::string("struct gui::timeline::state::").size();
    boost::statechart::state_machine< Machine, Idle >::process_event(evt);
    LOG_INFO <<  "[event=" << std::string(typeid( evt ).name()).substr(pos) << "][newstate=" << (state_begin() == state_end() ? "???" : std::string(typeid( *state_begin() ).name()).substr(pos)) << "]";
}

void Machine::onMotion(wxMouseEvent& event)
{
    getKeyboard().update(event);
    getMouse().update(event);
    process_event(EvMotion());
    event.Skip();
}

void Machine::onLeftDown(wxMouseEvent& event)
{
    getKeyboard().update(event);
    getMouse().update(event);
    getMouse().leftDown();
    getTimeline().SetFocus();
    process_event(EvLeftDown());
    event.Skip();
}

void Machine::onLeftUp(wxMouseEvent& event)
{
    getKeyboard().update(event);
    getMouse().update(event);
    process_event(EvLeftUp());
    event.Skip();
}

void Machine::onLeftDouble(wxMouseEvent& event)
{
    getKeyboard().update(event);
    getMouse().update(event);
    getMouse().leftDown();
    process_event(EvLeftDouble());
    event.Skip();
}

void Machine::onMiddleDown(wxMouseEvent& event)
{
    getKeyboard().update(event);
    getMouse().update(event);
    process_event(EvMiddleDown());
    event.Skip();
}

void Machine::onMiddleUp(wxMouseEvent& event)
{
    getKeyboard().update(event);
    getMouse().update(event);
    process_event(EvMiddleUp());
    event.Skip();
}

void Machine::onMiddleDouble(wxMouseEvent& event)
{
    getKeyboard().update(event);
    getMouse().update(event);
    process_event(EvMiddleDouble());
    event.Skip();
}

void Machine::onRightDown(wxMouseEvent& event)
{
    getKeyboard().update(event);
    getMouse().update(event);
    getMouse().rightDown();
    getTimeline().SetFocus();
    process_event(EvRightDown());
    event.Skip();
}

void Machine::onRightUp(wxMouseEvent& event)
{
    getKeyboard().update(event);
    getMouse().update(event);
    process_event(EvRightUp());
    event.Skip();
}

void Machine::onRightDouble(wxMouseEvent& event)
{
    getKeyboard().update(event);
    getMouse().update(event);
    getMouse().rightDown();
    process_event(EvRightDouble());
    event.Skip();
}

void Machine::onEnter(wxMouseEvent& event)
{
    getKeyboard().update(event);
    getMouse().update(event);
    process_event(EvEnter());
    event.Skip();
}

void Machine::onLeave(wxMouseEvent& event)
{
    getKeyboard().update(event);
    getMouse().update(event);
    process_event(EvLeave());
    event.Skip();
}

void Machine::onWheel(wxMouseEvent& event)
{
    getKeyboard().update(event);
    getMouse().update(event);

    // Zooming/Scrolling can be done in any state // todo check: won't this cause problems for the stored virtual position in Mouse class???
    int nSteps = event.GetWheelRotation() / event.GetWheelDelta();
    if (event.ControlDown())
    {
        getZoom().change(nSteps);
    }
    else if (event.ShiftDown())
    {
        int x;
        int y;
        getTimeline().GetViewStart(&x,&y);
        static const int sHorizontalScrollfactor = 100;
        getTimeline().Scroll(x - nSteps * sHorizontalScrollfactor,y);
    }
    else
    {
        // Only when this event is 'unhandled' here, the original scrolling
        // behaviour should be done.
        event.Skip();
    }

    process_event(EvWheel());
}

void Machine::onKeyDown(wxKeyEvent& event)
{
    getKeyboard().update(event); // todo improve logging of events: params
    process_event(EvKeyDown(event.ControlDown(), event.ShiftDown(), event.AltDown(), event.GetUnicodeKey(), event.GetKeyCode(), getTimeline().getScrolling().getVirtualPosition(event.GetPosition())));
    event.Skip();
}

void Machine::onKeyUp(wxKeyEvent& event)
{
    getKeyboard().update(event);
    process_event(EvKeyUp(event.ControlDown(), event.ShiftDown(), event.AltDown(), event.GetUnicodeKey(), event.GetKeyCode(), getTimeline().getScrolling().getVirtualPosition(event.GetPosition())));
    event.Skip();
}

void Machine::onCaptureLost(wxMouseCaptureLostEvent& event)
{
    LOG_DEBUG;
};

void Machine::onCaptureChanged(wxMouseCaptureChangedEvent& event)
{
    LOG_DEBUG;
};

void Machine::onZoomChanged( timeline::ZoomChangeEvent& event )
{
    LOG_DEBUG;
    process_event(EvZoomChanged(event));
    event.Skip();
}

void Machine::onScrollChanged( timeline::ScrollChangeEvent& event )
{
    LOG_DEBUG;
    process_event(EvScrollChanged(event));
    event.Skip();
}

void Machine::onPlaybackActive(PlaybackActiveEvent& event)
{
    LOG_DEBUG;
    process_event(EvPlaybackChanged(event.getValue()));
    event.Skip();
}

}}} // namespace