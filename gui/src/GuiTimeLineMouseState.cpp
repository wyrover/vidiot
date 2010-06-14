#include "GuiTimeLineMouseState.h"

#include <set>
#include <boost/foreach.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include "GuiTimeLine.h"
#include "GuiTimeLineClip.h"
#include "UtilLog.h"

namespace mousestate {

//////////////////////////////////////////////////////////////////////////
// MOUSE EVENTS
//////////////////////////////////////////////////////////////////////////

template< class MostDerived >
struct EvMouse : bs::event< MostDerived >
{
    EvMouse(wxPoint position, GuiTimeLineClipPtr clip, wxMouseEvent& event)
        :   mPosition(position)
        ,   mClip(clip)
        ,   mWxEvent(event)
    {

    }
    const wxPoint mPosition;
    const GuiTimeLineClipPtr mClip;
    const wxMouseEvent& mWxEvent;
};


struct EvMouse1Down : EvMouse<EvMouse1Down> { EvMouse1Down  (wxPoint position, GuiTimeLineClipPtr clip, wxMouseEvent& originalevent) : EvMouse(position, clip, originalevent) {} };
struct EvMouse1Up   : EvMouse<EvMouse1Up>   { EvMouse1Up    (wxPoint position, GuiTimeLineClipPtr clip, wxMouseEvent& originalevent) : EvMouse(position, clip, originalevent) {} };
struct EvMouse1Drag : EvMouse<EvMouse1Drag> { EvMouse1Drag  (wxPoint position, GuiTimeLineClipPtr clip, wxMouseEvent& originalevent) : EvMouse(position, clip, originalevent) {} };

template< class MostDerived >
std::ostream& operator<< (std::ostream& os, const mousestate::EvMouse< MostDerived >& obj)
{
    os << typeid(obj).name() << ',' << obj.mPosition << ',' << obj.mClip;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// MEMBERS ACESSIBLE BY ALL STATES
//////////////////////////////////////////////////////////////////////////

struct GlobalState
{
    GlobalState()
        :   DragStartPosition(-1,-1)
    {
    }
    wxPoint DragStartPosition;
    std::set<GuiTimeLineClipPtr> SelectedClips;
};

//////////////////////////////////////////////////////////////////////////
// STATES
//////////////////////////////////////////////////////////////////////////

struct TestDragStart;
struct Dragging;

//////////////////////////////////////////////////////////////////////////
// MACHINE
//////////////////////////////////////////////////////////////////////////

Machine::Machine(GuiTimeLine& tl)
    :   timeline(tl)
{
    globals = new GlobalState();
    initiate();
}

Machine::~Machine()
{
    delete globals;
}

void Machine::processMouseEvent(wxPoint virtualposition, GuiTimeLineClipPtr clip, wxMouseEvent& event)
{
    if (event.LeftDown())
    {
        process_event(EvMouse1Down(virtualposition, clip, event));

    }
    else if (event.Dragging())
    {
        if (event.LeftIsDown())
        {
            process_event(mousestate::EvMouse1Drag(virtualposition, clip, event));
        }
    }
    else if (event.LeftUp())
    {
        process_event(mousestate::EvMouse1Up(virtualposition, clip, event));
    }
}

//////////////////////////////////////////////////////////////////////////

struct AwaitingAction : bs::simple_state< AwaitingAction, Machine >
{
    typedef boost::mpl::list<
        bs::custom_reaction< EvMouse1Down >,
        bs::custom_reaction< EvMouse1Up >,
        bs::custom_reaction< EvMouse1Drag >
    > reactions;

    AwaitingAction() // entry
    {
        LOG_DEBUG; 
    }
    ~AwaitingAction() // exit
    { 
        LOG_DEBUG; 
    }
    bs::result react( const EvMouse1Down& evt )
    {
        VAR_DEBUG(evt);
        if (evt.mClip)
        {
            BOOST_FOREACH( GuiTimeLineClipPtr clip,outermost_context().globals->SelectedClips )
            {
                clip->setSelected(false);
            }
            outermost_context().globals->SelectedClips.clear();

            evt.mClip->setSelected(true);
            outermost_context().globals->SelectedClips.insert(evt.mClip);

            outermost_context().globals->DragStartPosition = evt.mPosition;
            return transit<TestDragStart>();
        }
        else
        {
            outermost_context().timeline.moveCursorOnUser(evt.mPosition.x);
        }
        return discard_event();
    }
    bs::result react( const EvMouse1Up& evt )
    {
        VAR_DEBUG(evt);
        return discard_event();
    }
    bs::result react( const EvMouse1Drag& evt )
    {
        VAR_DEBUG(evt);
        outermost_context().timeline.moveCursorOnUser(evt.mPosition.x);
        return discard_event();
    }
};

//////////////////////////////////////////////////////////////////////////

struct TestDragStart : bs::simple_state< TestDragStart, Machine >
{
    typedef boost::mpl::list<
        bs::custom_reaction< EvMouse1Down >,
        bs::custom_reaction< EvMouse1Up >,
        bs::custom_reaction< EvMouse1Drag >
    > reactions;

    TestDragStart() // entry
    {
        LOG_DEBUG; 
    }
    ~TestDragStart() // exit
    { 
        LOG_DEBUG; 
    }
    bs::result react( const EvMouse1Down& evt )
    {
        VAR_DEBUG(evt);
        return discard_event();
    }
    bs::result react( const EvMouse1Up& evt )
    {
        VAR_DEBUG(evt);
        return transit<AwaitingAction>();
    }
    bs::result react( const EvMouse1Drag& evt )
    {
        VAR_DEBUG(evt);
        wxPoint diff = outermost_context().globals->DragStartPosition - evt.mPosition;
        static int tolerance = 2;
        if ((abs(diff.x) > tolerance) || (abs(diff.y) > tolerance))
        {
            BOOST_FOREACH( GuiTimeLineClipPtr clip,outermost_context().globals->SelectedClips )
            {
                clip->setBeingDragged(true);
            }
            outermost_context().timeline.beginDrag(evt.mPosition);
            return transit<Dragging>();
        }
        return discard_event();
    }
};

//////////////////////////////////////////////////////////////////////////

struct Dragging : bs::simple_state< Dragging, Machine >
{
    typedef boost::mpl::list<
        bs::custom_reaction< EvMouse1Down >,
        bs::custom_reaction< EvMouse1Up >,
        bs::custom_reaction< EvMouse1Drag >
    > reactions;

    Dragging() // entry
    {
        LOG_DEBUG; 
    }
    ~Dragging() // exit
    { 
        LOG_DEBUG; 
    }
    bs::result react( const EvMouse1Down& evt )
    {
        VAR_DEBUG(evt);
        return discard_event();
    }
    bs::result react( const EvMouse1Up& evt )
    {
        VAR_DEBUG(evt);
        BOOST_FOREACH( GuiTimeLineClipPtr clip,outermost_context().globals->SelectedClips )
        {
            clip->setBeingDragged(false);
        }
        outermost_context().timeline.endDrag(evt.mPosition);
        return transit<AwaitingAction>();
    }
    bs::result react( const EvMouse1Drag& evt )
    {
        VAR_DEBUG(evt);
        outermost_context().timeline.moveDrag(evt.mPosition);
        return discard_event();
    }
};

} // namespace