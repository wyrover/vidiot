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
#include "UtilInt.h"

namespace gui { namespace timeline { namespace cmd {
    class EditClipDetails;
}}}

namespace gui { namespace timeline { namespace state {

struct EvLeftUp;
struct EvMotion;
struct EvLeave;
struct EvKeyDown;

struct MoveKeyFrame
    :   public TimeLineState< MoveKeyFrame, Machine >
{
    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    MoveKeyFrame( my_context ctx );

    virtual ~MoveKeyFrame();

    typedef boost::mpl::list<
        boost::statechart::custom_reaction< EvMotion >,
        boost::statechart::custom_reaction< EvLeftUp >,
        boost::statechart::custom_reaction< EvLeave >,
        boost::statechart::custom_reaction< EvKeyDown >
    > reactions;

    //////////////////////////////////////////////////////////////////////////
    // EVENTS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result react( const EvLeftUp& evt );
    boost::statechart::result react( const EvMotion& evt );
    boost::statechart::result react( const EvLeave& evt);
    boost::statechart::result react( const EvKeyDown& evt);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::IClipPtr mClip = nullptr;
    model::IClipPtr mClone = nullptr;
    size_t mKeyFrameIndex{ 0 };
    pts mKeyFramePosition{ 0 };
    std::pair<pts, pts> mBoundaries;

    cmd::EditClipDetails* mEditCommand = nullptr;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    boost::statechart::result abort();
};

}}} // namespace
