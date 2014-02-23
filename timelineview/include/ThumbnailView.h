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

#ifndef THUMBNAIL_VIEW_H
#define THUMBNAIL_VIEW_H

#include "View.h"

namespace gui { namespace timeline {

class ThumbnailView
    :   public View
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    ThumbnailView(model::IClipPtr clip, View* parent);
    virtual ~ThumbnailView();

    //////////////////////////////////////////////////////////////////////////
    // VIEW
    //////////////////////////////////////////////////////////////////////////

    pixel getX() const override;
    pixel getY() const override;
    pixel getW() const override;
    pixel getH() const override;

    void invalidateRect() override;

    void draw(wxDC& dc, const wxRegion& region, const wxPoint& offset) const override;
    void drawForDragging(wxPoint position, int height, wxDC& dc) const;

private:

    model::VideoClipPtr mVideoClip;
    mutable boost::optional<wxBitmap> mBitmap;
    mutable boost::optional<pixel> mW;
    mutable boost::optional<pixel> mH;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void determineSize() const;

    void draw(wxBitmap& bitmap) const;
};

}} // namespace

#endif
