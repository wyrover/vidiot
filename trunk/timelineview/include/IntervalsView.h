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

#ifndef INTERVALS_VIEW_H
#define INTERVALS_VIEW_H

#include "View.h"

namespace gui { namespace timeline {

class IntervalsView
    :   public View
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    IntervalsView(View* parent);
    virtual ~IntervalsView();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    wxSize requiredSize() const override;  ///< @see View::requiredSize()

    //////////////////////////////////////////////////////////////////////////
    // DRAWING
    //////////////////////////////////////////////////////////////////////////

    /// This view draws onto the bitmap of the parent view, and not onto its
    /// own bitmap. Rationale: the intervals are depicted 'on top' of the sequence,
    /// and are not drawn onto a separate region in the timeline.
    void draw(wxDC& dc) const;

    void refreshInterval(PtsInterval interval);

private:

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    wxRect makeRect(PtsInterval interval) const;
    void draw(wxBitmap& bitmap) const override; ///< @see View::draw()
};

}} // namespace

#endif // INTERVALS_VIEW_H