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

struct AwaitingAction;

//////////////////////////////////////////////////////////////////////////
// MACHINE
//////////////////////////////////////////////////////////////////////////

struct Machine : bs::state_machine< Machine, AwaitingAction >
{
    Machine(GuiTimeLine& tl);
    ~Machine();
    GuiTimeLine& timeline;
    GlobalState* globals;
    void processMouseEvent(wxPoint virtualposition, GuiTimeLineClipPtr clip, wxMouseEvent& event);
};

} // namespace

#endif // GUI_TIME_LINE_MOUSE_STATE_H