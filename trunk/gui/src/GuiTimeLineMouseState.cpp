#include "GuiTimeLineMouseState.h"

#include <set>
#include <boost/foreach.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include "GuiTimeLine.h"
#include "GuiTimeLineClip.h"
#include "UtilLog.h"

namespace mousestate {

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
// INITIALIZATION
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
            if (!evt.mClip->isSelected())
            {
                evt.mClip->setSelected(true);
                context<Machine>().globals->SelectedClips.insert(evt.mClip);
            }
            else
            {
                evt.mClip->setSelected(false);
                context<Machine>().globals->SelectedClips.erase(evt.mClip);
            }

            context<Machine>().globals->DragStartPosition = evt.mPosition;
            return transit<TestDragStart>();
        }
        else
        {
            context<Machine>().timeline.moveCursorOnUser(evt.mPosition.x);
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
        context<Machine>().timeline.moveCursorOnUser(evt.mPosition.x);
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
        wxPoint diff = context<Machine>().globals->DragStartPosition - evt.mPosition;
        static int tolerance = 2;
        if ((abs(diff.x) > tolerance) || (abs(diff.y) > tolerance))
        {
            BOOST_FOREACH( GuiTimeLineClipPtr clip, context<Machine>().globals->SelectedClips )
            {
                clip->setBeingDragged(true);
            }
            context<Machine>().timeline.updateBitmap();
            context<Machine>().timeline.Refresh();
            context<Machine>().timeline.beginDrag(evt.mPosition);
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
        context<Machine>().timeline.endDrag(evt.mPosition);
        return transit<AwaitingAction>();
    }
    bs::result react( const EvMouse1Drag& evt )
    {
        VAR_DEBUG(evt);
        context<Machine>().timeline.moveDrag(evt.mPosition);
        return discard_event();
    }
};

} // namespace