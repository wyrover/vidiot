// Copyright 2013 Eric Raijmakers.
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

#ifndef STATE_RIGHTDOWN_H
#define STATE_RIGHTDOWN_H

#include "State.h"

namespace gui { namespace timeline { namespace state {

struct EvRightDown;
struct EvRightUp;
struct EvMotion;
struct EvLeave;
struct EvKeyDown;

struct StateRightDown
    :   public TimeLineState< StateRightDown, Machine >
{
    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    StateRightDown( my_context ctx );

    virtual ~StateRightDown();

    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvRightDown >,
        boost::statechart::custom_reaction< EvRightUp >,
        boost::statechart::custom_reaction< EvMotion >,
        boost::statechart::custom_reaction< EvLeave >,
        boost::statechart::custom_reaction< EvKeyDown >
    > reactions;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result react( const EvRightDown& evt );
    boost::statechart::result react( const EvRightUp& evt );
    boost::statechart::result react( const EvMotion& evt );
    boost::statechart::result react( const EvLeave& evt );
    boost::statechart::result react( const EvKeyDown& evt );

    static const wxString sTooltip;
};

}}} // namespace

#endif // STATE_RIGHTDOWN_H