#include "State.h"

#include "Timeline.h"
#include "StateIdle.h"

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
}

Machine::~Machine()
{
}

void Machine::OnMotion          (wxMouseEvent& event)  { process_event(EvMotion        (event, unscrolledPosition(event.GetPosition()))); event.Skip(); }
void Machine::OnLeftDown        (wxMouseEvent& event)  { process_event(EvLeftDown      (event, unscrolledPosition(event.GetPosition()))); event.Skip(); }
void Machine::OnLeftUp          (wxMouseEvent& event)  { process_event(EvLeftUp        (event, unscrolledPosition(event.GetPosition()))); event.Skip(); }
void Machine::OnLeftDouble      (wxMouseEvent& event)  { process_event(EvLeftDouble    (event, unscrolledPosition(event.GetPosition()))); event.Skip(); }
void Machine::OnMiddleDown      (wxMouseEvent& event)  { process_event(EvMiddleDown    (event, unscrolledPosition(event.GetPosition()))); event.Skip(); }
void Machine::OnMiddleUp        (wxMouseEvent& event)  { process_event(EvMiddleUp      (event, unscrolledPosition(event.GetPosition()))); event.Skip(); }
void Machine::OnMiddleDouble    (wxMouseEvent& event)  { process_event(EvMiddleDouble  (event, unscrolledPosition(event.GetPosition()))); event.Skip(); }
void Machine::OnRightDown       (wxMouseEvent& event)  { process_event(EvRightDown     (event, unscrolledPosition(event.GetPosition()))); event.Skip(); }
void Machine::OnRightUp         (wxMouseEvent& event)  { process_event(EvRightUp       (event, unscrolledPosition(event.GetPosition()))); event.Skip(); }
void Machine::OnRightDouble     (wxMouseEvent& event)  { process_event(EvRightDouble   (event, unscrolledPosition(event.GetPosition()))); event.Skip(); }
void Machine::OnEnter           (wxMouseEvent& event)  { process_event(EvEnter         (event, unscrolledPosition(event.GetPosition()))); event.Skip(); }
void Machine::OnLeave           (wxMouseEvent& event)  { process_event(EvLeave         (event, unscrolledPosition(event.GetPosition()))); event.Skip(); }
void Machine::OnWheel           (wxMouseEvent& event)  { process_event(EvWheel         (event, unscrolledPosition(event.GetPosition()))); event.Skip(); }
void Machine::OnKeyDown         (wxKeyEvent&   event)  { process_event(EvKeyDown       (event, unscrolledPosition(event.GetPosition()))); event.Skip(); }  
void Machine::OnKeyUp           (wxKeyEvent&   event)  { process_event(EvKeyUp         (event, unscrolledPosition(event.GetPosition()))); event.Skip(); }
void Machine::OnCaptureLost     (wxMouseCaptureLostEvent& event) {};
void Machine::OnCaptureChanged  (wxMouseCaptureChangedEvent& event) {};

wxPoint Machine::unscrolledPosition(wxPoint position) const
{
    wxPoint p;
    mTimeline.CalcUnscrolledPosition(position.x,position.y,&p.x,&p.y);
    return p;
}

}}} // namespace