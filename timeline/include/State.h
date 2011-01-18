#ifndef TIMELINE_STATE_H
#define TIMELINE_STATE_H

#include <wx/gdicmn.h>
#include <boost/statechart/event.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/state_machine.hpp>
#include "UtilLogWxwidgets.h"
#include "Part.h"

namespace gui { namespace timeline { namespace state {

//////////////////////////////////////////////////////////////////////////
// MEMBERS ACESSIBLE BY ALL STATES
//////////////////////////////////////////////////////////////////////////

/// Using this class as base for all state classes ensures that these 
/// states can access the various timeline parts in the same way 
/// as these parts. 
template < class STATE, class CONTEXT >
class TimeLineState
    :   public boost::statechart::state<STATE, CONTEXT >
    ,   protected Part
{
public:
    TimeLineState( my_context ctx ) : my_base( ctx ) {};
    ~TimeLineState() {};
protected:
    Timeline& getTimeline() 
    { 
        return outermost_context().mTimeline; 
    };
};

/// Using this class as base for all state classes ensures that these 
/// states can access the various timeline parts in the same way 
/// as these parts. 
/// This version has an intial inner state.
template < class STATE, class CONTEXT, class INITIALINNERSTATE >
class TimeLineStateInner
    :   public boost::statechart::state<STATE, CONTEXT, INITIALINNERSTATE >
    ,   protected Part
{
public:
    TimeLineStateInner( my_context ctx ) : my_base( ctx ) {};
    ~TimeLineStateInner() {};
protected:
    Timeline& getTimeline() 
    { 
        return outermost_context().mTimeline; 
    };
};

//////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATION OF ALL STATES FOR TIMELINE
//////////////////////////////////////////////////////////////////////////

class Always;
class Idle;
class Dragging;
class MovingCursor;
class Playing;
class TestDragStart;

//////////////////////////////////////////////////////////////////////////
// MACHINE
//////////////////////////////////////////////////////////////////////////

class Machine
    :   public boost::statechart::state_machine< Machine, Always >
{
public:
    Machine(Timeline& tl);
    ~Machine();
    Timeline& mTimeline;

private:

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

    /// Converts a wxevent position to a virtual position on the
    /// timeline's bitmap buffer (which may be scrolled).
    wxPoint unscrolledPosition(wxPoint position) const;
};

//////////////////////////////////////////////////////////////////////////
// MOUSE EVENTS
//////////////////////////////////////////////////////////////////////////

template< class MostDerived >
struct EvMouse : boost::statechart::event< MostDerived >
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
std::ostream& operator<< (std::ostream& os, const EvMouse< MostDerived >& obj)
{
    os  << typeid(obj).name() << ',' // This typeid is required to distinguish the various 'react' methods
        << obj.mPosition << ','
        << obj.mWxEvent;
    return os;
}

struct EvMotion         : EvMouse<EvMotion>         { EvMotion      (wxMouseEvent& wxevt, wxPoint pos) : EvMouse< EvMotion >        (wxevt, pos) {} };
struct EvLeftDown       : EvMouse<EvLeftDown>       { EvLeftDown    (wxMouseEvent& wxevt, wxPoint pos) : EvMouse< EvLeftDown >      (wxevt, pos) {} };
struct EvLeftUp         : EvMouse<EvLeftUp>         { EvLeftUp      (wxMouseEvent& wxevt, wxPoint pos) : EvMouse< EvLeftUp >        (wxevt, pos) {} };
struct EvLeftDouble     : EvMouse<EvLeftDouble>     { EvLeftDouble  (wxMouseEvent& wxevt, wxPoint pos) : EvMouse< EvLeftDouble >    (wxevt, pos) {} };
struct EvMiddleDown     : EvMouse<EvMiddleDown>     { EvMiddleDown  (wxMouseEvent& wxevt, wxPoint pos) : EvMouse< EvMiddleDown >    (wxevt, pos) {} };
struct EvMiddleUp       : EvMouse<EvMiddleUp>       { EvMiddleUp    (wxMouseEvent& wxevt, wxPoint pos) : EvMouse< EvMiddleUp >      (wxevt, pos) {} };
struct EvMiddleDouble   : EvMouse<EvMiddleDouble>   { EvMiddleDouble(wxMouseEvent& wxevt, wxPoint pos) : EvMouse< EvMiddleDouble >  (wxevt, pos) {} };
struct EvRightDown      : EvMouse<EvRightDown>      { EvRightDown   (wxMouseEvent& wxevt, wxPoint pos) : EvMouse< EvRightDown >     (wxevt, pos) {} };
struct EvRightUp        : EvMouse<EvRightUp>        { EvRightUp     (wxMouseEvent& wxevt, wxPoint pos) : EvMouse< EvRightUp >       (wxevt, pos) {} };
struct EvRightDouble    : EvMouse<EvRightDouble>    { EvRightDouble (wxMouseEvent& wxevt, wxPoint pos) : EvMouse< EvRightDouble >   (wxevt, pos) {} };
struct EvEnter          : EvMouse<EvEnter>          { EvEnter       (wxMouseEvent& wxevt, wxPoint pos) : EvMouse< EvEnter >         (wxevt, pos) {} };
struct EvLeave          : EvMouse<EvLeave>          { EvLeave       (wxMouseEvent& wxevt, wxPoint pos) : EvMouse< EvLeave >         (wxevt, pos) {} };
struct EvWheel          : EvMouse<EvWheel>          { EvWheel       (wxMouseEvent& wxevt, wxPoint pos) : EvMouse< EvWheel >         (wxevt, pos) {} };

//////////////////////////////////////////////////////////////////////////
// KEY EVENTS
//////////////////////////////////////////////////////////////////////////

template< class MostDerived >
struct EvKey : boost::statechart::event< MostDerived >
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
std::ostream& operator<< (std::ostream& os, const EvKey< MostDerived >& obj)
{
    os  << typeid(obj).name() << ',' // This typeid is required to distinguish the various 'react' methods
        << obj.mPosition << ','
        << obj.mWxEvent;
    return os;
}

struct EvKeyDown : EvKey<EvKeyDown> { EvKeyDown (wxKeyEvent& wxevt, wxPoint pos) : EvKey<EvKeyDown> (wxevt, pos) {} };
struct EvKeyUp   : EvKey<EvKeyUp>   { EvKeyUp   (wxKeyEvent& wxevt, wxPoint pos) : EvKey<EvKeyUp>   (wxevt, pos) {} };

}}} // namespace

#endif // TIMELINE_STATE_H
