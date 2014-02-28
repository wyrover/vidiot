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

#ifndef CLIP_VIEW_H
#define CLIP_VIEW_H

#include "View.h"

namespace model{
class EventDragClip;
class EventSelectClip;
class DebugEventRenderProgress;
}

namespace gui { namespace timeline {
    struct PointerPositionInfo;

class ClipView
    :   public View
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    ClipView(const model::IClipPtr& clip, View* parent);
    virtual ~ClipView();

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
    //  GET & SET
    //////////////////////////////////////////////////////////////////////////

    model::IClipPtr getClip();

    pts getLeftPts() const;         ///< \return left pts value of the view. That includes the part oerlapping with any preceding transition (which is not the case for model::IClip)
    pts getRightPts() const;        ///< \return right pts value of the view. That includes the part oerlapping with any succeeding transition (which is not the case for model::IClip)

    pixel getLeftPixel() const;     ///< \return left position in pixels
    pixel getRightPixel() const;    ///< \return right position in pixels

    pixel getShift() const;          ///< \return shift to accommocate repositioning during shift-drag

    void getPositionInfo(const wxPoint& position, PointerPositionInfo& info) const;

    void update();

    //////////////////////////////////////////////////////////////////////////
    // DRAW
    //////////////////////////////////////////////////////////////////////////

    void drawForDragging(const wxPoint& position, int height, wxDC& dc, wxDC& dcMask) const;

    //////////////////////////////////////////////////////////////////////////
    // MODEL EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onClipDragged(model::EventDragClip& event);
    void onClipSelected(model::EventSelectClip& event);
    void onGenerationProgress( model::DebugEventRenderProgress& event);

private:

    model::IClipPtr mClip;

    pts mBeginAddition; ///< if >0 then this area is (temporarily, during editing) added. if <0 then it is removed from the clip.

    mutable boost::optional<wxBitmap> mBitmap;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Actual drawing implementation. Reused for drawing clips when dragging.
    /// \param drawDraggedClips indicates if clips currently being dragged must be drawn or left empty
    /// \param drawNotDraggedClips indicates if clips currently NOT being dragged  must be drawn or left empty
    void draw(wxBitmap& bitmap, bool drawDraggedClips, bool drawNotDraggedClips) const;

};

}} // namespace

#endif
