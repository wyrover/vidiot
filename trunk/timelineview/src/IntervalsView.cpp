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

#include "IntervalsView.h"

#include "Constants.h"
#include "Intervals.h"
#include "Layout.h"
#include "Scrolling.h"
#include "SequenceView.h"
#include "Timeline.h"
#include "Zoom.h"

#include "UtilLog.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

IntervalsView::IntervalsView(View* parent)
    :   View(parent)
{
    VAR_DEBUG(this);
    getIntervals().setView(this);
}

IntervalsView::~IntervalsView()
{
    VAR_DEBUG(this);
    getIntervals().setView(0);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

wxSize IntervalsView::requiredSize() const
{
    return wxSize(0,0);
}

//////////////////////////////////////////////////////////////////////////
// DRAWING
//////////////////////////////////////////////////////////////////////////

void IntervalsView::draw(wxDC& dc) const
{
    PtsIntervals intervals = getIntervals().getIntervalsForDrawing();

    dc.SetPen(*wxGREY_PEN);
    wxBrush b(*wxLIGHT_GREY,wxBRUSHSTYLE_CROSSDIAG_HATCH);
    dc.SetBrush(b);

    for ( PtsInterval i : intervals )
    {
        dc.DrawRectangle(makeRect(i));
    }
}

void IntervalsView::refreshInterval(PtsInterval interval)
{
    wxRect r(makeRect(interval));

    // Adjust for scrolling
    r.x -= getScrolling().getOffset().x;
    r.y -= getScrolling().getOffset().y;

    // enlargement to ensure that the vertical black end line of adjacent rects will be (re)drawn. Typical use: remove in the middle of an interval.
    r.x -= 1;
    r.width += 2;
    getTimeline().RefreshRect(r);
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

wxRect IntervalsView::makeRect(PtsInterval interval) const
{
    PixelInterval pixels( getZoom().ptsToPixels(interval.lower()), getZoom().ptsToPixels(interval.upper()) );
    return wxRect(pixels.lower(),0,pixels.upper() - pixels.lower() + 1,getSequenceView().getSize().GetHeight());
}

void IntervalsView::draw(wxBitmap& bitmap) const
{
    FATAL("Use the wcDC draw method.");
}

}} // namespace