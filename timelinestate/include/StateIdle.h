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

#include "Enums.h"
#include "State.h"

namespace gui { namespace timeline { namespace state {

struct EvCut;
struct EvCopy;
struct EvPaste;
struct EvLeftDown;
struct EvLeftDouble;
struct EvRightDown;
struct EvRightDouble;
struct EvMotion;
struct EvKeyDown;
struct EvDragEnter;
struct EvPlaybackChanged;

struct Idle
    :   public TimeLineState< Idle, Machine >
{
    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Idle( my_context ctx );

    virtual ~Idle();

    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvCut >,
        boost::statechart::custom_reaction< EvCopy >,
        boost::statechart::custom_reaction< EvPaste >,
        boost::statechart::custom_reaction< EvLeftDown >,
        boost::statechart::custom_reaction< EvLeftDouble >,
        boost::statechart::custom_reaction< EvRightDown >,
        boost::statechart::custom_reaction< EvRightDouble >,
        boost::statechart::custom_reaction< EvMotion >,
        boost::statechart::custom_reaction< EvKeyDown >,
        boost::statechart::custom_reaction< EvDragEnter >,
        boost::statechart::custom_reaction< EvPlaybackChanged >
    > reactions;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result react(const EvCut& evt);
    boost::statechart::result react(const EvCopy& evt);
    boost::statechart::result react(const EvPaste& evt);
    boost::statechart::result react(const EvLeftDown& evt);
    boost::statechart::result react(const EvLeftDouble& evt);
    boost::statechart::result react(const EvRightDown& evt);
    boost::statechart::result react(const EvRightDouble& evt);
    boost::statechart::result react(const EvMotion& evt);
    boost::statechart::result react(const EvKeyDown& evt);
    boost::statechart::result react(const EvDragEnter& evt);
    boost::statechart::result react(const EvPlaybackChanged& evt);

private:

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void updateMouseCursor();

    boost::statechart::result start();

    boost::statechart::result leftDown();
    boost::statechart::result rightDown();

    void addTransition(model::TransitionType type);
};

}}} // namespace
