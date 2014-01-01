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

#ifndef SELECT_INTERVALS_H
#define SELECT_INTERVALS_H

#include "Part.h"
#include "UtilInt.h"

namespace gui { namespace timeline {

class EventTimelineCursorMoved;
class IntervalsView;

class Intervals
    :   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Intervals(Timeline* timeline);
    virtual ~Intervals();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    void setView(IntervalsView* view);
    IntervalsView& getView();
    const IntervalsView& getView() const;

    bool isEmpty(); ///< @return true if there is at least one marked interval
    PtsIntervals get();
    void set(PtsIntervals region);
    void removeAll();

    void addBeginMarker();
    void addEndMarker();

    void startToggle();
    void endToggle();
    bool toggleIsAddition() const;

    void update(pts newCursorPosition); ///< To be called when the cursor is moved.
    void change(PtsInterval interval, bool add); ///< To be called for the undo/redo mechanism.
    void clear(); ///< Clear all marked intervals.

    PtsIntervals getIntervalsForDrawing() const;

    //////////////////////////////////////////////////////////////////////////
    // ACTIONS ON THE MARKED AREAS
    //////////////////////////////////////////////////////////////////////////

    void deleteMarked();
    void deleteUnmarked();

    void deleteEmpty();
    void deleteEmptyClip(model::IClipPtr empty);

private:

    IntervalsView* mView;

    PtsIntervals mIntervals;

    bool mNewIntervalActive;
    pts mNewIntervalBegin;
    pts mNewIntervalEnd;

    bool mToggleActive;
    pts mToggleBegin;
    pts mToggleEnd;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    pts determineSnap(pts position) const;
    void removeRegionUsedByClips(model::SequencePtr sequence, PtsIntervals& intervals);

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

}} // namespace

// Workaround needed to prevent compile-time errors (mpl_assertion_in_line...) with gcc
//#include  <boost/preprocessor/slot/counter.hpp>
//#include BOOST____PP_UPDATE_COUNTER()
//#line BOOST_____PP_COUNTER
BOOST_CLASS_VERSION(gui::timeline::Intervals, 1)

#endif // SELECT_INTERVALS_H