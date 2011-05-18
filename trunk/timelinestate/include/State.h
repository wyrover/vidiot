#ifndef TIMELINE_STATE_H
#define TIMELINE_STATE_H

#include <wx/gdicmn.h>
#include <boost/statechart/state.hpp>
#include <boost/statechart/state_machine.hpp>
#include "Part.h"

namespace gui { namespace timeline { 
    
class ZoomChangeEvent;
class ScrollChangeEvent;

namespace state {

//////////////////////////////////////////////////////////////////////////
// MEMBERS ACESSIBLE BY ALL STATES
//////////////////////////////////////////////////////////////////////////

/// Using this class as base for all state classes ensures that these 
/// states can access the various timeline parts.
template < class STATE, class CONTEXT >
class TimeLineState
    :   public boost::statechart::state<STATE, CONTEXT >
    ,   protected Part
{
public:
    TimeLineState( my_context ctx )
        :   my_base( ctx )
        ,   Part()
    {
    };
    ~TimeLineState()
    {
    };
protected:
    Timeline& getTimeline() 
    { 
        return outermost_context().getTimeline();
    };
};

//////////////////////////////////////////////////////////////////////////
// MACHINE
//////////////////////////////////////////////////////////////////////////

class Idle;

class Machine
    :   public boost::statechart::state_machine< Machine, Idle >
    ,   public Part
{
public:
    Machine(Timeline& tl);
    ~Machine();

private:

    void onMotion           (wxMouseEvent& event);
    void onLeftDown         (wxMouseEvent& event);
    void onLeftUp           (wxMouseEvent& event);
    void onLeftDouble       (wxMouseEvent& event);
    void onMiddleDown       (wxMouseEvent& event);
    void onMiddleUp         (wxMouseEvent& event);
    void onMiddleDouble     (wxMouseEvent& event);
    void onRightDown        (wxMouseEvent& event);
    void onRightUp          (wxMouseEvent& event);
    void onRightDouble      (wxMouseEvent& event);
    void onEnter            (wxMouseEvent& event);
    void onLeave            (wxMouseEvent& event);
    void onWheel            (wxMouseEvent& event);
    void onKeyDown          (wxKeyEvent&   event);
    void onKeyUp            (wxKeyEvent&   event);
    void onCaptureLost      (wxMouseCaptureLostEvent& event);
    void onCaptureChanged   (wxMouseCaptureChangedEvent& event);

    void onZoomChanged      (timeline::ZoomChangeEvent& event );
    void onScrollChanged    (timeline::ScrollChangeEvent& event);
};

}}} // namespace

#endif // TIMELINE_STATE_H
