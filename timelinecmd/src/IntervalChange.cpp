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

#include "IntervalChange.h"

#include "Intervals.h"
#include "Timeline.h"

namespace gui { namespace timeline { namespace cmd {

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

    IntervalChange::IntervalChange(const model::SequencePtr& sequence, const PtsInterval& interval, bool add)
:   ATimelineCommand(sequence)
,   mInterval(interval)
,   mAdd(add)
{
    VAR_INFO(this)(interval);
    if (mAdd)
    {
        mCommandName = _("Add new interval selection");
    }
    else
    {
        mCommandName = _("Remove new interval selection");
    }
}

IntervalChange::~IntervalChange()
{
}

//////////////////////////////////////////////////////////////////////////
// COMMAND
//////////////////////////////////////////////////////////////////////////

bool IntervalChange::Do()
{
    VAR_INFO(this);
    getTimeline().getIntervals().change(mInterval,mAdd);
    return true;
}

bool IntervalChange::Undo()
{
    VAR_INFO(this);
    getTimeline().getIntervals().change(mInterval,!mAdd);
    return true;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const IntervalChange& obj)
{
    os << static_cast<const ATimelineCommand&>(obj) << '|' << obj.mInterval << '|' << obj.mAdd;
    return os;
}

}}} // namespace