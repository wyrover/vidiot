// Copyright 2013-2015 Eric Raijmakers.
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

#pragma once

#include "EventKey.h"
#include "EventMouse.h"
#include "Part.h"

namespace gui
{
    class PlaybackActiveEvent;
    class PlaybackPositionEvent;
}

namespace gui { namespace timeline {

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
    TimeLineState( typename boost::statechart::state<STATE, CONTEXT >::my_context ctx )
        :   boost::statechart::state<STATE, CONTEXT >::my_base( ctx )
        ,   Part()
    {
    };
    virtual ~TimeLineState()
    {
    };
protected:
    Timeline& getTimeline() override
    {
        return this->outermost_context().getTimeline();
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

    void onMotion           (wxMouseEvent& event);
    void handleMotion       (EvMotion& event);

    void onLeftDown         (wxMouseEvent& event);
    void handleLeftDown     (EvLeftDown& event);

    void onLeftUp           (wxMouseEvent& event);
    void handleLeftUp       (EvLeftUp& event);

    void onLeftDouble       (wxMouseEvent& event);
    void handleLeftDouble   (EvLeftDouble& event);

    void onMiddleDown       (wxMouseEvent& event);
    void handleMiddleDown   (EvMiddleDown& event);

    void onMiddleUp         (wxMouseEvent& event);
    void handleMiddleUp     (EvMiddleUp& event);

    void onMiddleDouble     (wxMouseEvent& event);
    void handleMiddleDouble (EvMiddleDouble& event);

    void onRightDown        (wxMouseEvent& event);
    void handleRightDown    (EvRightDown& event);

    void onRightUp          (wxMouseEvent& event);
    void handleRightUp      (EvRightUp& event);

    void onRightDouble      (wxMouseEvent& event);
    void handleRightDouble  (EvRightDouble& event);

    void onEnter            (wxMouseEvent& event);
    void handleEnter        (EvEnter& event);

    void onLeave            (wxMouseEvent& event);
    void handleLeave        (EvLeave& event);

    void onWheel            (wxMouseEvent& event);

    void onKeyDown          (wxKeyEvent& event);
    void handleKeyDown      (EvKeyDown& event);

    void onKeyUp            (wxKeyEvent&   event);
    void handleKeyUp        (EvKeyUp& event);

    void onCaptureLost      (wxMouseCaptureLostEvent& event);

    void onCaptureChanged   (wxMouseCaptureChangedEvent& event);

    void onPlaybackActive(PlaybackActiveEvent& event);
    void onPlaybackPosition(PlaybackPositionEvent& event);

};

}}} // namespace
