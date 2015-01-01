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

#ifndef INTERVAL_CHANGE_H
#define INTERVAL_CHANGE_H

#include "ATimelineCommand.h"
#include "UtilInt.h"

namespace gui { namespace timeline { namespace command {

class IntervalChange
    :   public ATimelineCommand
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// \param Intervals the object to which the change must be made. Is a shared ptr to avoid exceptions in undo handling when closing the timeline.
    /// \param interval interval to be added/removed
    /// \param add true if interval must be added, false if interval must be removed
    IntervalChange(const model::SequencePtr& sequence, const PtsInterval& interval, bool add);

    virtual ~IntervalChange();

    //////////////////////////////////////////////////////////////////////////
    // WXWIDGETS DO/UNDO INTERFACE
    //////////////////////////////////////////////////////////////////////////

    bool Do() override;
    bool Undo() override;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    PtsInterval mInterval;
    bool mAdd;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<(std::ostream& os, const IntervalChange& obj);
};

}}} // namespace

#endif
