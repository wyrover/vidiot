#include "State.h"

#include "GuiTimeLine.h"
#include "StateIdle.h"

namespace gui { namespace timeline { namespace state {

//////////////////////////////////////////////////////////////////////////
// MACHINE
//////////////////////////////////////////////////////////////////////////

Machine::Machine(GuiTimeLine& tl)
:   timeline(tl)
{
    globals = new GlobalState(tl);
    initiate();

    timeline.Bind(wxEVT_MOTION,                 &Machine::OnMotion,         this);
    timeline.Bind(wxEVT_LEFT_DOWN,              &Machine::OnLeftDown,       this);
    timeline.Bind(wxEVT_LEFT_UP,                &Machine::OnLeftUp,         this);
    timeline.Bind(wxEVT_LEFT_DCLICK,            &Machine::OnLeftDouble,     this);
    timeline.Bind(wxEVT_MIDDLE_DOWN,            &Machine::OnMiddleDown,     this);
    timeline.Bind(wxEVT_MIDDLE_UP,              &Machine::OnMiddleUp,       this);
    timeline.Bind(wxEVT_MIDDLE_DCLICK,          &Machine::OnMiddleDouble,   this);
    timeline.Bind(wxEVT_RIGHT_DOWN,             &Machine::OnRightDown,      this);
    timeline.Bind(wxEVT_RIGHT_UP,               &Machine::OnRightUp,        this);
    timeline.Bind(wxEVT_RIGHT_DCLICK,           &Machine::OnRightDouble,    this);
    timeline.Bind(wxEVT_ENTER_WINDOW,           &Machine::OnEnter,          this);
    timeline.Bind(wxEVT_LEAVE_WINDOW,           &Machine::OnLeave,          this);
    timeline.Bind(wxEVT_MOUSEWHEEL,             &Machine::OnWheel,          this);
    timeline.Bind(wxEVT_KEY_DOWN,               &Machine::OnKeyDown,        this);
    timeline.Bind(wxEVT_KEY_UP,                 &Machine::OnKeyUp,          this);
    timeline.Bind(wxEVT_MOUSE_CAPTURE_LOST,     &Machine::OnCaptureLost,    this);
    timeline.Bind(wxEVT_MOUSE_CAPTURE_CHANGED,  &Machine::OnCaptureChanged, this);
}

Machine::~Machine()
{
    delete globals;
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
    timeline.CalcUnscrolledPosition(position.x,position.y,&p.x,&p.y);
    return p;
}

}}} // namespace