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
    showAnimation();

    getTimeline().beginTransaction();
    Revert();

    getTimeline().getIntervals().removeAll();
    std::set< model::IClips > removedInAllTracks = splitTracksAndFindClipsToBeRemoved(mRemoved);
    BOOST_FOREACH( model::IClips remove, removedInAllTracks )
    {
        removeClips(remove);
    }
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
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void TrimIntervals::showAnimation()
{
    std::set< model::IClips > removedInAllTracks = splitTracksAndFindClipsToBeRemoved(mRemoved);

    model::IClips mEmpties;
    BOOST_FOREACH( model::IClips remove, removedInAllTracks )
    {
        mEmpties.push_back(replaceWithEmpty(remove));
    }

    wxSafeYield(); // Show update progress, but do not allow user input
    boost::this_thread::sleep(boost::posix_time::milliseconds(200));

    animatedTrimEmpty(mEmpties);
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