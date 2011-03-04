#include "State.h"

#include "Timeline.h"
#include "StateIdle.h"
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

    mTimeline.Bind(wxEVT_MOTION,                 &Machine::OnMotion,         this);
    mTimeline.Bind(wxEVT_LEFT_DOWN,              &Machine::OnLeftDown,       this);
    mTimeline.Bind(wxEVT_LEFT_UP,                &Machine::OnLeftUp,         this);
    mTimeline.Bind(wxEVT_LEFT_DCLICK,            &Machine::OnLeftDouble,     this);
    mTimeline.Bind(wxEVT_MIDDLE_DOWN,            &Machine::OnMiddleDown,     this);
    mTimeline.Bind(wxEVT_MIDDLE_UP,              &Machine::OnMiddleUp,       this);
    mTimeline.Bind(wxEVT_MIDDLE_DCLICK,          &Machine::OnMiddleDouble,   this);
    mTimeline.Bind(wxEVT_RIGHT_DOWN,             &Machine::OnRightDown,      this);
    mTimeline.Bind(wxEVT_RIGHT_UP,               &Machine::OnRightUp,        this);
    mTimeline.Bind(wxEVT_RIGHT_DCLICK,           &Machine::OnRightDouble,    this);
    mTimeline.Bind(wxEVT_ENTER_WINDOW,           &Machine::OnEnter,          this);
    mTimeline.Bind(wxEVT_LEAVE_WINDOW,           &Machine::OnLeave,          this);
    mTimeline.Bind(wxEVT_MOUSEWHEEL,             &Machine::OnWheel,          this);
    mTimeline.Bind(wxEVT_KEY_DOWN,               &Machine::OnKeyDown,        this);
    mTimeline.Bind(wxEVT_KEY_UP,                 &Machine::OnKeyUp,          this);
    mTimeline.Bind(wxEVT_MOUSE_CAPTURE_LOST,     &Machine::OnCaptureLost,    this);
    mTimeline.Bind(wxEVT_MOUSE_CAPTURE_CHANGED,  &Machine::OnCaptureChanged, this);

    VAR_DEBUG(this);
}

Machine::~Machine()
{
    VAR_DEBUG(this);

    mTimeline.Unbind(wxEVT_MOTION,                 &Machine::OnMotion,         this);
    mTimeline.Unbind(wxEVT_LEFT_DOWN,              &Machine::OnLeftDown,       this);
    mTimeline.Unbind(wxEVT_LEFT_UP,                &Machine::OnLeftUp,         this);
    mTimeline.Unbind(wxEVT_LEFT_DCLICK,            &Machine::OnLeftDouble,     this);
    mTimeline.Unbind(wxEVT_MIDDLE_DOWN,            &Machine::OnMiddleDown,     this);
    mTimeline.Unbind(wxEVT_MIDDLE_UP,              &Machine::OnMiddleUp,       this);
    mTimeline.Unbind(wxEVT_MIDDLE_DCLICK,          &Machine::OnMiddleDouble,   this);
    mTimeline.Unbind(wxEVT_RIGHT_DOWN,             &Machine::OnRightDown,      this);
    mTimeline.Unbind(wxEVT_RIGHT_UP,               &Machine::OnRightUp,        this);
    mTimeline.Unbind(wxEVT_RIGHT_DCLICK,           &Machine::OnRightDouble,    this);
    mTimeline.Unbind(wxEVT_ENTER_WINDOW,           &Machine::OnEnter,          this);
    mTimeline.Unbind(wxEVT_LEAVE_WINDOW,           &Machine::OnLeave,          this);
    mTimeline.Unbind(wxEVT_MOUSEWHEEL,             &Machine::OnWheel,          this);
    mTimeline.Unbind(wxEVT_KEY_DOWN,               &Machine::OnKeyDown,        this);
    mTimeline.Unbind(wxEVT_KEY_UP,                 &Machine::OnKeyUp,          this);
    mTimeline.Unbind(wxEVT_MOUSE_CAPTURE_LOST,     &Machine::OnCaptureLost,    this);
    mTimeline.Unbind(wxEVT_MOUSE_CAPTURE_CHANGED,  &Machine::OnCaptureChanged, this);
}

void Machine::OnMotion          (wxMouseEvent& event)  { process_event(EvMotion        (event, mTimeline.getScrolling().getPhysicalPosition(event.GetPosition()))); event.Skip(); }
void Machine::OnLeftDown        (wxMouseEvent& event)  { process_event(EvLeftDown      (event, mTimeline.getScrolling().getPhysicalPosition(event.GetPosition()))); event.Skip(); }
void Machine::OnLeftUp          (wxMouseEvent& event)  { process_event(EvLeftUp        (event, mTimeline.getScrolling().getPhysicalPosition(event.GetPosition()))); event.Skip(); }
void Machine::OnLeftDouble      (wxMouseEvent& event)  { process_event(EvLeftDouble    (event, mTimeline.getScrolling().getPhysicalPosition(event.GetPosition()))); event.Skip(); }
void Machine::OnMiddleDown      (wxMouseEvent& event)  { process_event(EvMiddleDown    (event, mTimeline.getScrolling().getPhysicalPosition(event.GetPosition()))); event.Skip(); }
void Machine::OnMiddleUp        (wxMouseEvent& event)  { process_event(EvMiddleUp      (event, mTimeline.getScrolling().getPhysicalPosition(event.GetPosition()))); event.Skip(); }
void Machine::OnMiddleDouble    (wxMouseEvent& event)  { process_event(EvMiddleDouble  (event, mTimeline.getScrolling().getPhysicalPosition(event.GetPosition()))); event.Skip(); }
void Machine::OnRightDown       (wxMouseEvent& event)  { process_event(EvRightDown     (event, mTimeline.getScrolling().getPhysicalPosition(event.GetPosition()))); event.Skip(); }
void Machine::OnRightUp         (wxMouseEvent& event)  { process_event(EvRightUp       (event, mTimeline.getScrolling().getPhysicalPosition(event.GetPosition()))); event.Skip(); }
void Machine::OnRightDouble     (wxMouseEvent& event)  { process_event(EvRightDouble   (event, mTimeline.getScrolling().getPhysicalPosition(event.GetPosition()))); event.Skip(); }
void Machine::OnEnter           (wxMouseEvent& event)  { process_event(EvEnter         (event, mTimeline.getScrolling().getPhysicalPosition(event.GetPosition()))); event.Skip(); }
void Machine::OnLeave           (wxMouseEvent& event)  { process_event(EvLeave         (event, mTimeline.getScrolling().getPhysicalPosition(event.GetPosition()))); event.Skip(); }
void Machine::OnWheel           (wxMouseEvent& event)  { process_event(EvWheel         (event, mTimeline.getScrolling().getPhysicalPosition(event.GetPosition()))); } // NOT: event.Skip(); See handling of this event in Always state
void Machine::OnKeyDown         (wxKeyEvent&   event)  { process_event(EvKeyDown       (event, mTimeline.getScrolling().getPhysicalPosition(event.GetPosition()))); event.Skip(); }  
void Machine::OnKeyUp           (wxKeyEvent&   event)  { process_event(EvKeyUp         (event, mTimeline.getScrolling().getPhysicalPosition(event.GetPosition()))); event.Skip(); }
void Machine::OnCaptureLost     (wxMouseCaptureLostEvent& event) {};
void Machine::OnCaptureChanged  (wxMouseCaptureChangedEvent& event) {};

}}} // namespace