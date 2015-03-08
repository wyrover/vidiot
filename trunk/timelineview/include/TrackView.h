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

#ifndef TRACK_VIEW_H
#define TRACK_VIEW_H

#include "View.h"

namespace model {
class EventClipsReplaced;
class EventHeightChanged;
}

namespace gui { namespace timeline {

struct PointerPositionInfo;

class TrackView
    :   public View
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    TrackView(const model::TrackPtr& track, View* parent);
    virtual ~TrackView();

    model::TrackPtr getTrack() const;

    /// Draw the selected clips of this track only.
    /// This is required for drawing these clips when dragging.
    /// The height parameter is required for the case where this
    /// track is dragged over another track which has another height.
    void drawForDragging(const wxPoint& position, int height, wxDC& dc, wxDC& dcMask) const;

    //////////////////////////////////////////////////////////////////////////
    // VIEW
    //////////////////////////////////////////////////////////////////////////

    pixel getX() const override;
    pixel getY() const override;
    pixel getW() const override;
    pixel getH() const override;

    void invalidateRect() override;

    void draw(wxDC& dc, const wxRegion& region, const wxPoint& offset) const override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    void getPositionInfo(const wxPoint& position, PointerPositionInfo& info) const;

    //////////////////////////////////////////////////////////////////////////
    // MODEL EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onClipsReplaced(model::EventClipsReplaced& event);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::TrackPtr mTrack;
    mutable boost::optional<pixel> mY;
    typedef std::map<pixel, model::IClipPtr> ClipLookupMap;
    mutable boost::shared_ptr<ClipLookupMap> mClips;
};

}} // namespace

#endif
