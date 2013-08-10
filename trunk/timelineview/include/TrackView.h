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

#ifndef TRACK_VIEW_H
#define TRACK_VIEW_H

#include "View.h"

namespace model {
class EventAddClips;
class EventRemoveClips;
class EventHeightChanged;
}

namespace gui { namespace timeline {

struct PointerPositionInfo;

class TrackView
    :   public View
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    TrackView(model::TrackPtr track, View* parent);
    virtual ~TrackView();

    model::TrackPtr getTrack() const;

    /// Draw the selected clips of this track only.
    /// This is required for drawing these clips when dragging.
    /// The height parameter is required for the case where this
    /// track is dragged over another track which has another height.
    void drawForDragging(wxPoint position, int height, wxDC& dc, wxDC& dcMask) const;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    void canvasResized(); ///< Must be called when the widget is resized

    wxSize requiredSize() const override;  ///< @see View::requiredSize()

    void getPositionInfo(wxPoint position, PointerPositionInfo& info) const;

    void onShiftChanged();

    //////////////////////////////////////////////////////////////////////////
    // MODEL EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onClipsAdded( model::EventAddClips& event );
    void onClipsRemoved( model::EventRemoveClips& event );
    void onHeightChanged( model::EventHeightChanged& event );

private:

    model::TrackPtr mTrack;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void draw(wxBitmap& bitmap) const override; ///< @see View::draw()
};

}} // namespace

#endif // TRACK_VIEW_H