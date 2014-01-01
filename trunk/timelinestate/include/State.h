// Copyright 2013,2014 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#ifndef TIMELINE_STATE_H
#define TIMELINE_STATE_H

#include "Part.h"

namespace gui { class PlaybackActiveEvent; }

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
    virtual ~TimeLineState()
    {
    };
protected:
    Timeline& getTimeline() override
    {
        return outermost_context().getTimeline();
    };
};

//////////////////////////////////////////////////////////////////////////
// MACHINE
//////////////////////////////////////////////////////////////////////////

struct Starting;

class Machine
    :   public boost::statechart::state_machine< Machine, Starting >
    ,   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Machine(Timeline& tl);
    virtual ~Machine();

    void start();

    //////////////////////////////////////////////////////////////////////////
    // BOOST STATECHART OVERRIDES
    //////////////////////////////////////////////////////////////////////////

    void unconsumed_event( const boost::statechart::event_base & evt );
    void process_event(const boost::statechart::event_base & evt );

    //////////////////////////////////////////////////////////////////////////
    // HANDLING EVENTS - PUBLIC FOR REUSE IN TEST CODE
    //////////////////////////////////////////////////////////////////////////

    bool processWheelEvent(int nSteps);

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

    void onPlaybackActive(PlaybackActiveEvent& event);

};

}}} // namespace

#endif // TIMELINE_STATE_H