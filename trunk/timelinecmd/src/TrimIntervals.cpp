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

#include "TrimIntervals.h"

#include "Intervals.h"
#include "Timeline.h"
#include "UtilLog.h"

namespace gui { namespace timeline { namespace command {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

TrimIntervals::TrimIntervals(model::SequencePtr sequence, PtsIntervals remove, wxString name)
    :   AClipEdit(sequence)
    ,   mIntervals(getTimeline().getIntervals().get())
    ,   mRemoved(remove)
{
    VAR_INFO(this)(mIntervals)(name);
    mCommandName = name;
}

TrimIntervals::~TrimIntervals()
{
}

//////////////////////////////////////////////////////////////////////////
// ACLIPEDIT INTERFACE
//////////////////////////////////////////////////////////////////////////

void TrimIntervals::initialize()
{
    animatedDeleteAndTrim(splitTracksAndFindClipsToBeRemoved(mRemoved));
    getTimeline().endTransaction();
}

void TrimIntervals::doExtraBefore()
{
    LOG_INFO;
    getTimeline().getIntervals().removeAll();
}

void TrimIntervals::undoExtraAfter()
{
    getTimeline().getIntervals().set(mIntervals);
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const TrimIntervals& obj )
{
    os << static_cast<const AClipEdit&>(obj) << '|' << obj.mIntervals << '|' << obj.mRemoved;
    return os;
}
}}} // namespace