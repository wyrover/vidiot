#include "State.h"

#include "Timeline.h"
#include "StateIdle.h"
#include "StateAlways.h"
#include "UtilLog.h"
#include "Scrolling.h"

namespace gui { namespace timeline { namespace state {

//////////////////////////////////////////////////////////////////////////
// MACHINE
//////////////////////////////////////////////////////////////////////////

Machine::Machine(Timeline& tl)
:   mTimeline(tl)
{
    initiate();

    mTimeline.Bind(wxEVT_MOTION,                 &Machine::onMotion,         this);
    mTimeline.Bind(wxEVT_LEFT_DOWN,              &Machine::onLeftDown,       this);
    mTimeline.Bind(wxEVT_LEFT_UP,                &Machine::onLeftUp,         this);
    mTimeline.Bind(wxEVT_LEFT_DCLICK,            &Machine::onLeftDouble,     this);
    mTimeline.Bind(wxEVT_MIDDLE_DOWN,            &Machine::onMiddleDown,     this);
    mTimeline.Bind(wxEVT_MIDDLE_UP,              &Machine::onMiddleUp,       this);
    mTimeline.Bind(wxEVT_MIDDLE_DCLICK,          &Machine::onMiddleDouble,   this);
    mTimeline.Bind(wxEVT_RIGHT_DOWN,             &Machine::onRightDown,      this);
    mTimeline.Bind(wxEVT_RIGHT_UP,               &Machine::onRightUp,        this);
    mTimeline.Bind(wxEVT_RIGHT_DCLICK,           &Machine::onRightDouble,    this);
    mTimeline.Bind(wxEVT_ENTER_WINDOW,           &Machine::onEnter,          this);
    mTimeline.Bind(wxEVT_LEAVE_WINDOW,           &Machine::onLeave,          this);
    mTimeline.Bind(wxEVT_MOUSEWHEEL,             &Machine::onWheel,          this);
    mTimeline.Bind(wxEVT_KEY_DOWN,               &Machine::onKeyDown,        this);
    mTimeline.Bind(wxEVT_KEY_UP,                 &Machine::onKeyUp,          this);
    mTimeline.Bind(wxEVT_MOUSE_CAPTURE_LOST,     &Machine::onCaptureLost,    this);
    mTimeline.Bind(wxEVT_MOUSE_CAPTURE_CHANGED,  &Machine::onCaptureChanged, this);

    VAR_DEBUG(this);
}

Machine::~Machine()
{
    VAR_DEBUG(this);

    mTimeline.Unbind(wxEVT_MOTION,                 &Machine::onMotion,         this);
    mTimeline.Unbind(wxEVT_LEFT_DOWN,              &Machine::onLeftDown,       this);
    mTimeline.Unbind(wxEVT_LEFT_UP,                &Machine::onLeftUp,         this);
    mTimeline.Unbind(wxEVT_LEFT_DCLICK,            &Machine::onLeftDouble,     this);
    mTimeline.Unbind(wxEVT_MIDDLE_DOWN,            &Machine::onMiddleDown,     this);
    mTimeline.Unbind(wxEVT_MIDDLE_UP,              &Machine::onMiddleUp,       this);
    mTimeline.Unbind(wxEVT_MIDDLE_DCLICK,          &Machine::onMiddleDouble,   this);
    mTimeline.Unbind(wxEVT_RIGHT_DOWN,             &Machine::onRightDown,      this);
    mTimeline.Unbind(wxEVT_RIGHT_UP,               &Machine::onRightUp,        this);
    mTimeline.Unbind(wxEVT_RIGHT_DCLICK,           &Machine::onRightDouble,    this);
    mTimeline.Unbind(wxEVT_ENTER_WINDOW,           &Machine::onEnter,          this);
    mTimeline.Unbind(wxEVT_LEAVE_WINDOW,           &Machine::onLeave,          this);
    mTimeline.Unbind(wxEVT_MOUSEWHEEL,             &Machine::onWheel,          this);
    mTimeline.Unbind(wxEVT_KEY_DOWN,               &Machine::onKeyDown,        this);
    mTimeline.Unbind(wxEVT_KEY_UP,                 &Machine::onKeyUp,          this);
    mTimeline.Unbind(wxEVT_MOUSE_CAPTURE_LOST,     &Machine::onCaptureLost,    this);
    mTimeline.Unbind(wxEVT_MOUSE_CAPTURE_CHANGED,  &Machine::onCaptureChanged, this);
}

void Machine::onMotion          (wxMouseEvent& event)  { process_event(EvMotion        (event, mTimeline.getScrolling().getVirtualPosition(event.GetPosition()))); event.Skip(); }
void Machine::onLeftDown        (wxMouseEvent& event)  { process_event(EvLeftDown      (event, mTimeline.getScrolling().getVirtualPosition(event.GetPosition()))); event.Skip(); }
void Machine::onLeftUp          (wxMouseEvent& event)  { process_event(EvLeftUp        (event, mTimeline.getScrolling().getVirtualPosition(event.GetPosition()))); event.Skip(); }
void Machine::onLeftDouble      (wxMouseEvent& event)  { process_event(EvLeftDouble    (event, mTimeline.getScrolling().getVirtualPosition(event.GetPosition()))); event.Skip(); }
void Machine::onMiddleDown      (wxMouseEvent& event)  { process_event(EvMiddleDown    (event, mTimeline.getScrolling().getVirtualPosition(event.GetPosition()))); event.Skip(); }
void Machine::onMiddleUp        (wxMouseEvent& event)  { process_event(EvMiddleUp      (event, mTimeline.getScrolling().getVirtualPosition(event.GetPosition()))); event.Skip(); }
void Machine::onMiddleDouble    (wxMouseEvent& event)  { process_event(EvMiddleDouble  (event, mTimeline.getScrolling().getVirtualPosition(event.GetPosition()))); event.Skip(); }
void Machine::onRightDown       (wxMouseEvent& event)  { process_event(EvRightDown     (event, mTimeline.getScrolling().getVirtualPosition(event.GetPosition()))); event.Skip(); }
void Machine::onRightUp         (wxMouseEvent& event)  { process_event(EvRightUp       (event, mTimeline.getScrolling().getVirtualPosition(event.GetPosition()))); event.Skip(); }
void Machine::onRightDouble     (wxMouseEvent& event)  { process_event(EvRightDouble   (event, mTimeline.getScrolling().getVirtualPosition(event.GetPosition()))); event.Skip(); }
void Machine::onEnter           (wxMouseEvent& event)  { process_event(EvEnter         (event, mTimeline.getScrolling().getVirtualPosition(event.GetPosition()))); event.Skip(); }
void Machine::onLeave           (wxMouseEvent& event)  { process_event(EvLeave         (event, mTimeline.getScrolling().getVirtualPosition(event.GetPosition()))); event.Skip(); }
void Machine::onWheel           (wxMouseEvent& event)  { process_event(EvWheel         (event, mTimeline.getScrolling().getVirtualPosition(event.GetPosition()))); } // NOT: event.Skip(); See handling of this event in Always state
void Machine::onKeyDown         (wxKeyEvent&   event)  { process_event(EvKeyDown       (event, mTimeline.getScrolling().getVirtualPosition(event.GetPosition()))); event.Skip(); }  
void Machine::onKeyUp           (wxKeyEvent&   event)  { process_event(EvKeyUp         (event, mTimeline.getScrolling().getVirtualPosition(event.GetPosition()))); event.Skip(); }
void Machine::onCaptureLost     (wxMouseCaptureLostEvent& event) {};
void Machine::onCaptureChanged  (wxMouseCaptureChangedEvent& event) {};

}}} // namespace