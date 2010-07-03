#ifndef GUI_TIME_LINE_MOUSE_STATE_H
#define GUI_TIME_LINE_MOUSE_STATE_H

#include <wx/gdicmn.h>
#include <wx/event.h>
#include <boost/statechart/event.hpp>
#include <boost/statechart/state_machine.hpp>
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "GuiPtr.h"

namespace bs = boost::statechart;

namespace mousestate {

//////////////////////////////////////////////////////////////////////////
// MEMBERS ACESSIBLE BY ALL STATES
//////////////////////////////////////////////////////////////////////////

struct GlobalState;

//////////////////////////////////////////////////////////////////////////
// STATES
//////////////////////////////////////////////////////////////////////////

struct Idle;
struct Stopped;
struct Playing;
struct TestDragStart;
struct MovingCursor;
struct Dragging;

//////////////////////////////////////////////////////////////////////////
// MACHINE
//////////////////////////////////////////////////////////////////////////

struct Machine 
    :   bs::state_machine< Machine, Idle >
{
    Machine(GuiTimeLine& tl);
    ~Machine();
    GuiTimeLine& timeline;
    GlobalState* globals;
    
    void OnMotion           (wxMouseEvent& event);
    void OnLeftDown         (wxMouseEvent& event);
    void OnLeftUp           (wxMouseEvent& event);
    void OnLeftDouble       (wxMouseEvent& event);
    void OnMiddleDown       (wxMouseEvent& event);
    void OnMiddleUp         (wxMouseEvent& event);
    void OnMiddleDouble     (wxMouseEvent& event);
    void OnRightDown        (wxMouseEvent& event);
    void OnRightUp          (wxMouseEvent& event);
    void OnRightDouble      (wxMouseEvent& event);
    void OnEnter            (wxMouseEvent& event);
    void OnLeave            (wxMouseEvent& event);
    void OnWheel            (wxMouseEvent& event);
    void OnKeyDown          (wxKeyEvent&   event);
    void OnKeyUp            (wxKeyEvent&   event);
    void OnCaptureLost      (wxMouseCaptureLostEvent& event);
    void OnCaptureChanged   (wxMouseCaptureChangedEvent& event);

    /**
     * Converts a wxevent position to a virtual position on the 
     * timeline's bitmap buffer (which may be scrolled).
     */
    wxPoint unscrolledPosition(wxPoint position) const;

};

} // namespace

#endif // GUI_TIME_LINE_MOUSE_STATE_H