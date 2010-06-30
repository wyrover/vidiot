#include "GuiTimeLineMouseState.h"

#include <set>
#include <wx/event.h>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include "GuiTimeLine.h"
#include "GuiTimeLineClip.h"
#include "GuiTimeLineTrack.h"
#include "GuiTimeLineDragImage.h"
#include "MousePointer.h"
#include "Selection.h"
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
    os  << typeid(obj).name() << ',' 
        << obj.mPosition << ','
        << obj.mWxEvent;
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
    EvKey(wxKeyEvent& wxevt, wxPoint pos)
        :   mPosition(pos)
        ,   mWxEvent(wxevt)
    {
    };
    const wxPoint mPosition;
    const wxKeyEvent& mWxEvent;
};

template< class MostDerived >
std::ostream& operator<< (std::ostream& os, const mousestate::EvKey< MostDerived >& obj)
{
    os  << typeid(obj).name() << ',' 
        << obj.mPosition << ','
        << obj.mWxEvent;
    return os;
}

struct EvKeyDown : EvKey<EvKeyDown> { EvKeyDown (wxKeyEvent& wxevt, wxPoint pos) : EvKey(wxevt, pos) {} };
struct EvKeyUp   : EvKey<EvKeyUp>   { EvKeyUp   (wxKeyEvent& wxevt, wxPoint pos) : EvKey(wxevt, pos) {} };

//////////////////////////////////////////////////////////////////////////
// MEMBERS ACESSIBLE BY ALL STATES
//////////////////////////////////////////////////////////////////////////

struct GlobalState
{
    GlobalState(GuiTimeLine& timeline)
        :   DragStartPosition(-1,-1)
        ,   DragImage(0)
        ,   mousepointer(timeline)
        ,   selection(timeline)
    {
    }
    wxPoint DragStartPosition;
    GuiTimeLineDragImage* DragImage;
    MousePointer mousepointer;
    Selection selection;

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

void Machine::OnMotion          (wxMouseEvent& event)  { process_event(EvMotion        (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnLeftDown        (wxMouseEvent& event)  { process_event(EvLeftDown      (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnLeftUp          (wxMouseEvent& event)  { process_event(EvLeftUp        (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnLeftDouble      (wxMouseEvent& event)  { process_event(EvLeftDouble    (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnMiddleDown      (wxMouseEvent& event)  { process_event(EvMiddleDown    (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnMiddleUp        (wxMouseEvent& event)  { process_event(EvMiddleUp      (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnMiddleDouble    (wxMouseEvent& event)  { process_event(EvMiddleDouble  (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnRightDown       (wxMouseEvent& event)  { process_event(EvRightDown     (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnRightUp         (wxMouseEvent& event)  { process_event(EvRightUp       (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnRightDouble     (wxMouseEvent& event)  { process_event(EvRightDouble   (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnEnter           (wxMouseEvent& event)  { process_event(EvEnter         (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnLeave           (wxMouseEvent& event)  { process_event(EvLeave         (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnWheel           (wxMouseEvent& event)  { process_event(EvWheel         (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnKeyDown         (wxKeyEvent&   event)  { process_event(EvKeyDown       (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnKeyUp           (wxKeyEvent&   event)  { process_event(EvKeyUp         (event, unscrolledPosition(event.GetPosition())));   }
void Machine::OnCaptureLost     (wxMouseCaptureLostEvent& event) {};
void Machine::OnCaptureChanged  (wxMouseCaptureChangedEvent& event) {};

wxPoint Machine::unscrolledPosition(wxPoint position) const
{
    wxPoint p;
    timeline.CalcUnscrolledPosition(position.x,position.y,&p.x,&p.y);
    return p;
}

//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////

struct Idle : bs::simple_state< Idle, Machine >
{
    typedef boost::mpl::list<
        bs::custom_reaction< EvLeftDown >,
        bs::custom_reaction< EvMotion >
    > reactions;

    Idle() // entry
    {
        LOG_DEBUG; 
    }
    ~Idle() // exit
    { 
        LOG_DEBUG; 
    }
    bs::result react( const EvLeftDown& evt )
    {
        VAR_DEBUG(evt);
        GuiTimeLineClipPtr clip = outermost_context().timeline.findClip(evt.mPosition).get<0>();
        outermost_context().globals->selection.update(clip,evt.mWxEvent.ControlDown(),evt.mWxEvent.ShiftDown(),evt.mWxEvent.AltDown());
        if (clip)
        {
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
    bs::result react( const EvMotion& evt )
    {
        VAR_DEBUG(evt);
        MousePosition pos = outermost_context().globals->mousepointer.getLogicalPosition(evt.mPosition);
        MousePointerImage image = PointerNormal;
        switch (pos)
        {
        case MouseOnClipBegin:      image = evt.mWxEvent.ShiftDown() ? PointerTrimShiftBegin : PointerTrimBegin;    break;
        case MouseBetweenClips:     image = PointerMoveCut;     break;
        case MouseOnClipEnd:        image = evt.mWxEvent.ShiftDown() ? PointerTrimShiftEnd : PointerTrimEnd;    break;
        }
        outermost_context().globals->mousepointer.set(image);
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
        return transit<Idle>();
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
        return transit<Idle>();
    }
    bs::result react( const EvMotion& evt )
    {
        VAR_DEBUG(evt);
        wxPoint diff = outermost_context().globals->DragStartPosition - evt.mPosition;
        static int tolerance = 2;
        if ((abs(diff.x) > tolerance) || (abs(diff.y) > tolerance))
        {
            BOOST_FOREACH( GuiTimeLineClipPtr c, outermost_context().timeline.getClips() )
            {
                if (c->isSelected())
                {
                    c->setBeingDragged(true);
                }
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

    /** /todo handle mouse focus lost */
    Dragging() // entry
        :   mClip()
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

        BOOST_FOREACH( GuiTimeLineClipPtr c, outermost_context().timeline.getClips() )
        {
            c->setBeingDragged(false);
        }

        // End the drag operation
        GuiTimeLine& timeline = outermost_context().timeline;
        GuiTimeLineDragImage* dragimage = outermost_context().globals->DragImage;
        dragimage->Hide();
        dragimage->EndDrag();
        timeline.Refresh();
        delete outermost_context().globals->DragImage;
        outermost_context().globals->DragImage = 0;




        return transit<Idle>();
    }
    void showDropArea(wxPoint p)
    {
        GuiTimeLine& timeline = outermost_context().timeline;
        boost::tuple<GuiTimeLineTrackPtr,int> tt = timeline.findTrack(p.y);
        boost::tuple<GuiTimeLineClipPtr,int> cw = timeline.findClip(p);
        GuiTimeLineTrackPtr track = tt.get<0>();
        GuiTimeLineClipPtr clip = cw.get<0>();
        mClip = clip;

        if (track)
        {
            if (clip)
            {
                boost::tuple<int,int> clipbounds = track->findClipBounds(clip);

                int diffleft  = p.x - clipbounds.get<0>();
                int diffright = clipbounds.get<1>() - p.x;

                int xDrop = -1;
                if (diffleft < diffright)
                {
                    xDrop = clipbounds.get<0>() - 2;
                }
                else
                {
                    xDrop = clipbounds.get<1>() - 2;
                }
                timeline.showDropArea(wxRect(xDrop,tt.get<1>(),4,track->getBitmap().GetHeight())); 
            }
            else
            {
                timeline.showDropArea(wxRect(p.x,tt.get<1>(),4,track->getBitmap().GetHeight())); 
            }
        }
        else
        {
            timeline.showDropArea(wxRect(0,0,0,0));
        }
    }
    bs::result react( const EvMotion& evt )
    {
        VAR_DEBUG(evt);

        // Move the drag image
        GuiTimeLine& timeline = outermost_context().timeline;
        GuiTimeLineDragImage* dragimage = outermost_context().globals->DragImage;
        //dragimage->Hide();
        timeline.Refresh(false);
        timeline.Update();
        //dragimage->Show();
        dragimage->Move(evt.mPosition - timeline.getScrollOffset());

        showDropArea(evt.mPosition); 

        return discard_event();
    }


    GuiTimeLineClipPtr mClip;
};

} // namespace