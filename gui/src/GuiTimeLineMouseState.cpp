#include "GuiTimeLineMouseState.h"

#include <set>
#include <wx/event.h>
#include <boost/foreach.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/bind/placeholders.hpp>
#include <boost/function.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include "GuiTimeLine.h"
#include "GuiTimeLineClip.h"
#include "GuiTimeLineDragImage.h"
#include "UtilLog.h"

namespace mousestate {

//////////////////////////////////////////////////////////////////////////
// MOUSE EVENTS
//////////////////////////////////////////////////////////////////////////

template< class MostDerived >
struct EvMouse : bs::event< MostDerived >
{
    EvMouse(wxMouseEvent& wxevt, wxPoint pos)
        :   mPosition(pos)
        ,   mWxEvent(wxevt)
    {
    };
    const wxPoint mPosition;
    const wxMouseEvent& mWxEvent;
};

template< class MostDerived >
std::ostream& operator<< (std::ostream& os, const mousestate::EvMouse< MostDerived >& obj)
{
    os << typeid(obj).name() << ',' << obj.mPosition;
    return os;
}

struct EvMotion         : EvMouse<EvMotion>         { EvMotion      (wxMouseEvent& wxevt, wxPoint pos) : EvMouse(wxevt, pos) {} };
struct EvLeftDown       : EvMouse<EvLeftDown>       { EvLeftDown    (wxMouseEvent& wxevt, wxPoint pos) : EvMouse(wxevt, pos) {} };
struct EvLeftUp         : EvMouse<EvLeftUp>         { EvLeftUp      (wxMouseEvent& wxevt, wxPoint pos) : EvMouse(wxevt, pos) {} };
struct EvLeftDouble     : EvMouse<EvLeftDouble>     { EvLeftDouble  (wxMouseEvent& wxevt, wxPoint pos) : EvMouse(wxevt, pos) {} };
struct EvMiddleDown     : EvMouse<EvMiddleDown>     { EvMiddleDown  (wxMouseEvent& wxevt, wxPoint pos) : EvMouse(wxevt, pos) {} };
struct EvMiddleUp       : EvMouse<EvMiddleUp>       { EvMiddleUp    (wxMouseEvent& wxevt, wxPoint pos) : EvMouse(wxevt, pos) {} };
struct EvMiddleDouble   : EvMouse<EvMiddleDouble>   { EvMiddleDouble(wxMouseEvent& wxevt, wxPoint pos) : EvMouse(wxevt, pos) {} };
struct EvRightDown      : EvMouse<EvRightDown>      { EvRightDown   (wxMouseEvent& wxevt, wxPoint pos) : EvMouse(wxevt, pos) {} };
struct EvRightUp        : EvMouse<EvRightUp>        { EvRightUp     (wxMouseEvent& wxevt, wxPoint pos) : EvMouse(wxevt, pos) {} };
struct EvRightDouble    : EvMouse<EvRightDouble>    { EvRightDouble (wxMouseEvent& wxevt, wxPoint pos) : EvMouse(wxevt, pos) {} };
struct EvEnter          : EvMouse<EvEnter>          { EvEnter       (wxMouseEvent& wxevt, wxPoint pos) : EvMouse(wxevt, pos) {} };
struct EvLeave          : EvMouse<EvLeave>          { EvLeave       (wxMouseEvent& wxevt, wxPoint pos) : EvMouse(wxevt, pos) {} };
struct EvWheel          : EvMouse<EvWheel>          { EvWheel       (wxMouseEvent& wxevt, wxPoint pos) : EvMouse(wxevt, pos) {} };

template< class MostDerived >
struct EvKey : bs::event< MostDerived >
{
    EvKey(wxKeyEvent& wxevt)
        :   mWxEvent(wxevt)
    {
    };
    const wxKeyEvent& mWxEvent;
};

template< class MostDerived >
std::ostream& operator<< (std::ostream& os, const mousestate::EvKey< MostDerived >& obj)
{
    os << typeid(obj).name() << ',' << obj.mWxEvent.GetKeyCode();
    return os;
}

struct EvKeyDown : EvKey<EvKeyDown> { EvKeyDown (wxKeyEvent& wxevt) : EvKey(wxevt) {} };
struct EvKeyUp   : EvKey<EvKeyUp>   { EvKeyUp   (wxKeyEvent& wxevt) : EvKey(wxevt) {} };

//////////////////////////////////////////////////////////////////////////
// MEMBERS ACESSIBLE BY ALL STATES
//////////////////////////////////////////////////////////////////////////

struct GlobalState
{
    GlobalState()
        :   DragStartPosition(-1,-1)
        ,   DragImage(0)
    {
    }
    wxPoint DragStartPosition;
    GuiTimeLineDragImage* DragImage;
    std::set<GuiTimeLineClipPtr> SelectedClips;
};

//////////////////////////////////////////////////////////////////////////
// STATES
//////////////////////////////////////////////////////////////////////////

struct TestDragStart;
struct MovingCursor;
struct Dragging;

//////////////////////////////////////////////////////////////////////////
// MACHINE
//////////////////////////////////////////////////////////////////////////

Machine::Machine(GuiTimeLine& tl)
:   timeline(tl)
{
    globals = new GlobalState();
    initiate();

    timeline.Bind(wxEVT_MOTION,         &Machine::OnMotion,         this);
    timeline.Bind(wxEVT_LEFT_DOWN,      &Machine::OnLeftDown,       this);
    timeline.Bind(wxEVT_LEFT_UP,        &Machine::OnLeftUp,         this);
    timeline.Bind(wxEVT_LEFT_DCLICK,    &Machine::OnLeftDouble,     this);
    timeline.Bind(wxEVT_MIDDLE_DOWN,    &Machine::OnMiddleDown,     this);
    timeline.Bind(wxEVT_MIDDLE_UP,      &Machine::OnMiddleUp,       this);
    timeline.Bind(wxEVT_MIDDLE_DCLICK,  &Machine::OnMiddleDouble,   this);
    timeline.Bind(wxEVT_RIGHT_DOWN,     &Machine::OnRightDown,      this);
    timeline.Bind(wxEVT_RIGHT_UP,       &Machine::OnRightUp,        this);
    timeline.Bind(wxEVT_RIGHT_DCLICK,   &Machine::OnRightDouble,    this);
    timeline.Bind(wxEVT_ENTER_WINDOW,   &Machine::OnEnter,          this);
    timeline.Bind(wxEVT_LEAVE_WINDOW,   &Machine::OnLeave,          this);
    timeline.Bind(wxEVT_MOUSEWHEEL,     &Machine::OnWheel,          this);
    timeline.Bind(wxEVT_KEY_DOWN,       &Machine::OnKeyDown,        this);
    timeline.Bind(wxEVT_KEY_UP,         &Machine::OnKeyUp,          this);
}

Machine::~Machine()
{
    delete globals;
}

void Machine::OnMotion       (wxMouseEvent& event)  { process_event(EvMotion        (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnLeftDown     (wxMouseEvent& event)  { process_event(EvLeftDown      (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnLeftUp       (wxMouseEvent& event)  { process_event(EvLeftUp        (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnLeftDouble   (wxMouseEvent& event)  { process_event(EvLeftDouble    (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnMiddleDown   (wxMouseEvent& event)  { process_event(EvMiddleDown    (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnMiddleUp     (wxMouseEvent& event)  { process_event(EvMiddleUp      (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnMiddleDouble (wxMouseEvent& event)  { process_event(EvMiddleDouble  (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnRightDown    (wxMouseEvent& event)  { process_event(EvRightDown     (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnRightUp      (wxMouseEvent& event)  { process_event(EvRightUp       (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnRightDouble  (wxMouseEvent& event)  { process_event(EvRightDouble   (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnEnter        (wxMouseEvent& event)  { process_event(EvEnter         (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnLeave        (wxMouseEvent& event)  { process_event(EvLeave         (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnWheel        (wxMouseEvent& event)  { process_event(EvWheel         (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnKeyDown      (wxKeyEvent&   event)  { process_event(EvKeyDown       (event)); }
void Machine::OnKeyUp        (wxKeyEvent&   event)  { process_event(EvKeyUp         (event)); }

wxPoint Machine::unscrolledPosition(wxPoint position) const
{
    wxPoint p;
    timeline.CalcUnscrolledPosition(position.x,position.y,&p.x,&p.y);
    return p;
}

//////////////////////////////////////////////////////////////////////////

struct AwaitingAction : bs::simple_state< AwaitingAction, Machine >
{
    typedef boost::mpl::list<
        bs::custom_reaction< EvLeftDown >
    > reactions;

    AwaitingAction() // entry
    {
        LOG_DEBUG; 
    }
    ~AwaitingAction() // exit
    { 
        LOG_DEBUG; 
    }
    bs::result react( const EvLeftDown& evt )
    {
        VAR_DEBUG(evt);
        GuiTimeLineClipPtr clip = outermost_context().timeline.findClip(evt.mPosition);
        if (clip)
        {
            BOOST_FOREACH( GuiTimeLineClipPtr selectedclip,outermost_context().globals->SelectedClips )
            {
                selectedclip->setSelected(false);
            }
            outermost_context().globals->SelectedClips.clear();

            clip->setSelected(true);
            outermost_context().globals->SelectedClips.insert(clip);

            outermost_context().globals->DragStartPosition = evt.mPosition;
            return transit<TestDragStart>();
        }
        else
        {
            outermost_context().timeline.moveCursorOnUser(evt.mPosition.x);
            return transit<MovingCursor>();
        }
        return discard_event();
    }
};

//////////////////////////////////////////////////////////////////////////

struct MovingCursor : bs::simple_state< MovingCursor, Machine >
{
    typedef boost::mpl::list<
        bs::custom_reaction< EvLeftUp >,
        bs::custom_reaction< EvMotion >
    > reactions;

    MovingCursor() // entry
    {
        LOG_DEBUG; 
    }
    ~MovingCursor() // exit
    { 
        LOG_DEBUG; 
    }
    bs::result react( const EvLeftUp& evt )
    {
        VAR_DEBUG(evt);
        return transit<AwaitingAction>();
    }
    bs::result react( const EvMotion& evt )
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
        bs::custom_reaction< EvLeftUp >,
        bs::custom_reaction< EvMotion >
    > reactions;

    TestDragStart() // entry
    {
        LOG_DEBUG; 
    }
    ~TestDragStart() // exit
    { 
        LOG_DEBUG; 
    }
    bs::result react( const EvLeftUp& evt )
    {
        VAR_DEBUG(evt);
        return transit<AwaitingAction>();
    }
    bs::result react( const EvMotion& evt )
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

            // Begin the drag operation
            GuiTimeLine& timeline = outermost_context().timeline;
            outermost_context().globals->DragImage = new GuiTimeLineDragImage(&timeline, evt.mPosition);
            GuiTimeLineDragImage* dragimage = outermost_context().globals->DragImage;
            bool ok = dragimage->BeginDrag(dragimage->getHotspot(), &timeline, false);
            ASSERT(ok);
            timeline.Refresh(false);
            timeline.Update();
            dragimage->Move(evt.mPosition);
            dragimage->Show();

            //outermost_context().timeline.beginDrag(evt.mPosition);
            return transit<Dragging>();
        }
        return discard_event();
    }
};

//////////////////////////////////////////////////////////////////////////

struct Dragging : bs::simple_state< Dragging, Machine >
{
    typedef boost::mpl::list<
        bs::custom_reaction< EvLeftUp >,
        bs::custom_reaction< EvMotion >
    > reactions;

    Dragging() // entry
    {
        LOG_DEBUG; 
    }
    ~Dragging() // exit
    { 
        LOG_DEBUG; 
    }
    bs::result react( const EvLeftUp& evt )
    {
        VAR_DEBUG(evt);
        BOOST_FOREACH( GuiTimeLineClipPtr clip,outermost_context().globals->SelectedClips )
        {
            clip->setBeingDragged(false);
        }

        // End the drag operation
        GuiTimeLine& timeline = outermost_context().timeline;
        GuiTimeLineDragImage* dragimage = outermost_context().globals->DragImage;
        dragimage->Hide();
        dragimage->EndDrag();
        timeline.Refresh();
        delete outermost_context().globals->DragImage;
        outermost_context().globals->DragImage = 0;
        //        outermost_context().timeline.endDrag(evt.mPosition);

        return transit<AwaitingAction>();
    }
    bs::result react( const EvMotion& evt )
    {
        VAR_DEBUG(evt);

        // Move the drag image
        GuiTimeLine& timeline = outermost_context().timeline;
        GuiTimeLineDragImage* dragimage = outermost_context().globals->DragImage;
        dragimage->Hide();
        timeline.Refresh(false);
        timeline.Update();
        dragimage->Move(evt.mPosition);
        dragimage->Show();
        //        outermost_context().timeline.moveDrag(evt.mPosition);

        return discard_event();
    }
};

} // namespace