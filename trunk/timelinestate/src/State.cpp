#include "State.h"

#include "Timeline.h"
#include "StateIdle.h"
#include "UtilLog.h"
#include "Scrolling.h"
#include "MousePointer.h"
#include "Zoom.h"

namespace gui { namespace timeline { namespace state {

//////////////////////////////////////////////////////////////////////////
// MACHINE
//////////////////////////////////////////////////////////////////////////

Machine::Machine(Timeline& tl)
:   Part(&tl)
{
    initiate();

    getTimeline().Bind(wxEVT_MOTION,                 &Machine::onMotion,         this);
    getTimeline().Bind(wxEVT_LEFT_DOWN,              &Machine::onLeftDown,       this);
    getTimeline().Bind(wxEVT_LEFT_UP,                &Machine::onLeftUp,         this);
    getTimeline().Bind(wxEVT_LEFT_DCLICK,            &Machine::onLeftDouble,     this);
    getTimeline().Bind(wxEVT_MIDDLE_DOWN,            &Machine::onMiddleDown,     this);
    getTimeline().Bind(wxEVT_MIDDLE_UP,              &Machine::onMiddleUp,       this);
    getTimeline().Bind(wxEVT_MIDDLE_DCLICK,          &Machine::onMiddleDouble,   this);
    getTimeline().Bind(wxEVT_RIGHT_DOWN,             &Machine::onRightDown,      this);
    getTimeline().Bind(wxEVT_RIGHT_UP,               &Machine::onRightUp,        this);
    getTimeline().Bind(wxEVT_RIGHT_DCLICK,           &Machine::onRightDouble,    this);
    getTimeline().Bind(wxEVT_ENTER_WINDOW,           &Machine::onEnter,          this);
    getTimeline().Bind(wxEVT_LEAVE_WINDOW,           &Machine::onLeave,          this);
    getTimeline().Bind(wxEVT_MOUSEWHEEL,             &Machine::onWheel,          this);
    getTimeline().Bind(wxEVT_KEY_DOWN,               &Machine::onKeyDown,        this);
    getTimeline().Bind(wxEVT_KEY_UP,                 &Machine::onKeyUp,          this);
    getTimeline().Bind(wxEVT_MOUSE_CAPTURE_LOST,     &Machine::onCaptureLost,    this);
    getTimeline().Bind(wxEVT_MOUSE_CAPTURE_CHANGED,  &Machine::onCaptureChanged, this);

    VAR_DEBUG(this);
}

Machine::~Machine()
{
    VAR_DEBUG(this);

    getTimeline().Unbind(wxEVT_MOTION,                 &Machine::onMotion,         this);
    getTimeline().Unbind(wxEVT_LEFT_DOWN,              &Machine::onLeftDown,       this);
    getTimeline().Unbind(wxEVT_LEFT_UP,                &Machine::onLeftUp,         this);
    getTimeline().Unbind(wxEVT_LEFT_DCLICK,            &Machine::onLeftDouble,     this);
    getTimeline().Unbind(wxEVT_MIDDLE_DOWN,            &Machine::onMiddleDown,     this);
    getTimeline().Unbind(wxEVT_MIDDLE_UP,              &Machine::onMiddleUp,       this);
    getTimeline().Unbind(wxEVT_MIDDLE_DCLICK,          &Machine::onMiddleDouble,   this);
    getTimeline().Unbind(wxEVT_RIGHT_DOWN,             &Machine::onRightDown,      this);
    getTimeline().Unbind(wxEVT_RIGHT_UP,               &Machine::onRightUp,        this);
    getTimeline().Unbind(wxEVT_RIGHT_DCLICK,           &Machine::onRightDouble,    this);
    getTimeline().Unbind(wxEVT_ENTER_WINDOW,           &Machine::onEnter,          this);
    getTimeline().Unbind(wxEVT_LEAVE_WINDOW,           &Machine::onLeave,          this);
    getTimeline().Unbind(wxEVT_MOUSEWHEEL,             &Machine::onWheel,          this);
    getTimeline().Unbind(wxEVT_KEY_DOWN,               &Machine::onKeyDown,        this);
    getTimeline().Unbind(wxEVT_KEY_UP,                 &Machine::onKeyUp,          this);
    getTimeline().Unbind(wxEVT_MOUSE_CAPTURE_LOST,     &Machine::onCaptureLost,    this);
    getTimeline().Unbind(wxEVT_MOUSE_CAPTURE_CHANGED,  &Machine::onCaptureChanged, this);
}

void Machine::onMotion(wxMouseEvent& event)
{
    process_event(EvMotion(event, getTimeline().getScrolling().getVirtualPosition(event.GetPosition())));
    event.Skip();
}

void Machine::onLeftDown(wxMouseEvent& event)
{ 
    getWindow().SetFocus();
    wxPoint virtualPosition = getTimeline().getScrolling().getVirtualPosition(event.GetPosition());
    getMousePointer().setLeftDownPosition(virtualPosition);
    process_event(EvLeftDown(event, virtualPosition)); 
    event.Skip(); 
}

void Machine::onLeftUp(wxMouseEvent& event)
{ 
    process_event(EvLeftUp(event, getTimeline().getScrolling().getVirtualPosition(event.GetPosition()))); 
    event.Skip(); 
}

void Machine::onLeftDouble(wxMouseEvent& event)
{ 
    process_event(EvLeftDouble(event, getTimeline().getScrolling().getVirtualPosition(event.GetPosition()))); 
    event.Skip(); 
}

void Machine::onMiddleDown(wxMouseEvent& event)  
{ 
    process_event(EvMiddleDown(event, getTimeline().getScrolling().getVirtualPosition(event.GetPosition()))); 
    event.Skip(); 
}

void Machine::onMiddleUp(wxMouseEvent& event)  
{ 
    process_event(EvMiddleUp(event, getTimeline().getScrolling().getVirtualPosition(event.GetPosition()))); 
    event.Skip(); 
}

void Machine::onMiddleDouble(wxMouseEvent& event)  
{ 
    process_event(EvMiddleDouble(event, getTimeline().getScrolling().getVirtualPosition(event.GetPosition()))); 
    event.Skip(); 
}

void Machine::onRightDown(wxMouseEvent& event)  
{ 
    getWindow().SetFocus();
    wxPoint virtualPosition = getTimeline().getScrolling().getVirtualPosition(event.GetPosition());
    getMousePointer().setRightDownPosition(virtualPosition);
    process_event(EvRightDown(event, virtualPosition)); 
    event.Skip(); 
}

void Machine::onRightUp(wxMouseEvent& event)  
{ 
    process_event(EvRightUp(event, getTimeline().getScrolling().getVirtualPosition(event.GetPosition()))); 
    event.Skip(); 
}

void Machine::onRightDouble(wxMouseEvent& event)  
{ 
    process_event(EvRightDouble(event, getTimeline().getScrolling().getVirtualPosition(event.GetPosition()))); 
    event.Skip(); 
}

void Machine::onEnter(wxMouseEvent& event)  
{ 
    process_event(EvEnter(event, getTimeline().getScrolling().getVirtualPosition(event.GetPosition()))); 
    event.Skip(); 
}

void Machine::onLeave(wxMouseEvent& event)  
{ 
    process_event(EvLeave(event, getTimeline().getScrolling().getVirtualPosition(event.GetPosition()))); 
    event.Skip(); 
}

void Machine::onWheel(wxMouseEvent& event)  
{ 
    // Zooming/Scrolling can be done in any state
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
        static const int sHorizontalScrollfactor = 25;
        getTimeline().Scroll(x - nSteps * sHorizontalScrollfactor,y);
    }
    else
    {
        // Only when this event is 'unhandled' here, the original scrolling
        // behaviour should be done.
        event.Skip();
    }

    process_event(EvWheel(event, getTimeline().getScrolling().getVirtualPosition(event.GetPosition()))); 
}

void Machine::onKeyDown(wxKeyEvent&   event) 
{ 
    process_event(EvKeyDown(event, getTimeline().getScrolling().getVirtualPosition(event.GetPosition()))); 
    event.Skip(); 
}  

void Machine::onKeyUp(wxKeyEvent&   event)  
{ 
    process_event(EvKeyUp(event, getTimeline().getScrolling().getVirtualPosition(event.GetPosition()))); 
    event.Skip(); 
}

void Machine::onCaptureLost(wxMouseCaptureLostEvent& event) 
{
};

void Machine::onCaptureChanged(wxMouseCaptureChangedEvent& event) 
{
};

}}} // namespace