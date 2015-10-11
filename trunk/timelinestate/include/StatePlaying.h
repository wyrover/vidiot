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

#include "State.h"

namespace command {
class RootCommand;
}

namespace gui { namespace timeline { namespace state {

struct EvLeftDown;
struct EvRightDown;
struct EvKeyDown;
struct EvKeyUp;
struct EvPlaybackChanged;

struct Playing
    :   public TimeLineState< Playing, Machine >
{
    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Playing( my_context ctx );

    virtual ~Playing();

    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvLeftDown >,
        boost::statechart::custom_reaction< EvKeyDown >,
        boost::statechart::custom_reaction< EvKeyUp >,
        boost::statechart::custom_reaction< EvDragEnter >,
        boost::statechart::custom_reaction< EvPlaybackChanged >,
        boost::statechart::custom_reaction< EvPlaybackPositionChanged >
    > reactions;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result react( const EvLeftDown& evt );
    boost::statechart::result react( const EvKeyDown& evt);
    boost::statechart::result react( const EvKeyUp& evt);
    boost::statechart::result react( const EvDragEnter& evt);
    boost::statechart::result react( const EvPlaybackChanged& evt);
    boost::statechart::result react( const EvPlaybackPositionChanged& evt );

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mMakingNewSelection;
    boost::shared_ptr<EvKeyDown> mSubmitKeyEventOnStop;

    int mKeyCodeTriggeringStop; ///< The key code that triggered the stop of playback

    static const wxString sTooltip;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void triggerBegin();
    void triggerEnd();

};

}}} // namespace
