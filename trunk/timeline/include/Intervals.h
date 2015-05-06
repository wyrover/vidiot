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
    void set(const PtsIntervals& region);
    void removeAll();

    void addBeginMarker();
    void addEndMarker();

    void startToggle();
    void endToggle();
    bool toggleIsAddition() const;

    void update(pts newCursorPosition); ///< To be called when the cursor is moved.
    void change(const PtsInterval& interval, bool add); ///< To be called for the undo/redo mechanism.
    void clear(); ///< Clear all marked intervals.

    //////////////////////////////////////////////////////////////////////////
    // DRAWING
    //////////////////////////////////////////////////////////////////////////

    void draw(wxDC& dc, const wxRegion& region, const wxPoint& offset) const;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // ACTIONS ON THE MARKED AREAS
    //////////////////////////////////////////////////////////////////////////

    void deleteMarked();
    void deleteUnmarked();

    void deleteEmpty();
    void deleteEmptyClip(const model::IClipPtr& empty);

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
    void removeRegionUsedByClips(const model::SequencePtr& sequence, PtsIntervals& intervals);
    void refreshInterval(const PtsInterval& interval);
    wxRect makeRect(const PtsInterval& interval) const;

    //////////////////////////////////////////////////////////////////////////
    // SERIALIZATION
    //////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
};

}} // namespace

BOOST_CLASS_VERSION(gui::timeline::Intervals, 1)

#endif
