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

#ifndef STATE_DRAGGING_H
#define STATE_DRAGGING_H

#include "State.h"
#include "EventPart.h"

namespace gui { namespace timeline { namespace state {

struct EvLeftUp;
struct EvDragDrop;
struct EvMotion;
struct EvDragMove;
struct EvLeave;
struct EvDragEnd;
struct EvKeyDown;
struct EvKeyUp;

struct Dragging
    :   public TimeLineState< Dragging, Machine >
{
    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Dragging( my_context ctx );

    virtual ~Dragging();

    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvLeftUp >,
        boost::statechart::custom_reaction< EvDragDrop >,
        boost::statechart::custom_reaction< EvMotion >,
        boost::statechart::custom_reaction< EvDragMove >,
        boost::statechart::custom_reaction< EvLeave >,
        boost::statechart::custom_reaction< EvDragEnd >,
        boost::statechart::custom_reaction< EvKeyDown >,
        boost::statechart::custom_reaction< EvKeyUp >
    > reactions;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result react( const EvLeftUp& evt );
    boost::statechart::result react( const EvDragDrop& evt );
    boost::statechart::result react( const EvMotion& evt );
    boost::statechart::result react( const EvDragMove& evt );
    boost::statechart::result react( const EvLeave& evt );
    boost::statechart::result react( const EvDragEnd& evt );
    boost::statechart::result react( const EvKeyDown& evt );
    boost::statechart::result react( const EvKeyUp& evt );

    static const wxString sTooltip;
};

}}} // namespace

#endif
