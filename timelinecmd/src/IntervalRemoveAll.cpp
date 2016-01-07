// Copyright 2013-2016 Eric Raijmakers.
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

#include "IntervalRemoveAll.h"

#include "Intervals.h"
#include "Timeline.h"
#include "SequenceView.h"

namespace gui { namespace timeline { namespace cmd {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

IntervalRemoveAll::IntervalRemoveAll(const model::SequencePtr& sequence)
:   ATimelineCommand(sequence)
,   mIntervals(getTimeline().getIntervals().get())
{
    VAR_INFO(this);
    mCommandName = _("Remove all markers");
}

IntervalRemoveAll::~IntervalRemoveAll()
{
}

//////////////////////////////////////////////////////////////////////////
// COMMAND
//////////////////////////////////////////////////////////////////////////

bool IntervalRemoveAll::Do()
{
    VAR_INFO(this);
    getTimeline().getIntervals().removeAll();
    return true;
}

bool IntervalRemoveAll::Undo()
{
    VAR_INFO(this);
    getTimeline().getIntervals().set(mIntervals);
    return true;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const IntervalRemoveAll& obj)
{
    os << static_cast<const ATimelineCommand&>(obj) << '|' << obj.mIntervals;
    return os;
}

}}} // namespace