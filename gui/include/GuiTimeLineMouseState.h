#ifndef GUI_TIME_LINE_MOUSE_STATE_H
#define GUI_TIME_LINE_MOUSE_STATE_H

#include <wx/gdicmn.h>
#include <boost/statechart/event.hpp>
#include <boost/statechart/state_machine.hpp>
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "GuiPtr.h"

namespace bs = boost::statechart;

namespace mousestate {

struct GlobalState;

struct AwaitingAction;
struct Machine : bs::state_machine< Machine, AwaitingAction >
{
    Machine(GuiTimeLine& tl);
    ~Machine();
    GuiTimeLine& timeline;
    GlobalState* globals;
};

template< class MostDerived >
struct EvMouse : bs::event< MostDerived >
{
    EvMouse(wxPoint position, GuiTimeLineClipPtr clip)
        :   mPosition(position)
        ,   mClip(clip)
    {

    }
    const wxPoint mPosition;
    const GuiTimeLineClipPtr mClip;
};


struct EvMouse1Down : EvMouse<EvMouse1Down>
{
    EvMouse1Down(wxPoint position, GuiTimeLineClipPtr clip) : EvMouse(position, clip) {}
};

struct EvMouse1Up : EvMouse<EvMouse1Up>
{
    EvMouse1Up(wxPoint position, GuiTimeLineClipPtr clip) : EvMouse(position, clip) {}
};

struct EvMouse1Drag : EvMouse<EvMouse1Drag>
{
    EvMouse1Drag(wxPoint position, GuiTimeLineClipPtr clip) : EvMouse(position, clip) {}
};

template< class MostDerived >
std::ostream& operator<< (std::ostream& os, const mousestate::EvMouse< MostDerived >& obj)
{
    os << typeid(obj).name() << ',' << obj.mPosition << ',' << obj.mClip;
    return os;
}

} // namespace

#endif // GUI_TIME_LINE_MOUSE_STATE_H