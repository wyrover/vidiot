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

IntervalsView::IntervalsView(Timeline* timeline)
    :   View(timeline)
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
// POSITION/SIZE
//////////////////////////////////////////////////////////////////////////

pixel IntervalsView::getX() const
{
    return getParent().getX();
}

pixel IntervalsView::getY() const
{
    return 0;
}

pixel IntervalsView::getW() const
{
    return getParent().getW();
}

pixel IntervalsView::getH() const
{
    return getParent().getH();
}

void IntervalsView::invalidateRect()
{
}

void IntervalsView::draw(wxDC& dc, const wxRegion& region, const wxPoint& offset) const // todo move this to the intervals class entirely
{
    PtsIntervals intervals = getIntervals().getIntervalsForDrawing();

    if (!intervals.empty())
    {
        dc.SetPen(Layout::get().IntervalPen);
        dc.SetBrush(Layout::get().IntervalBrush);
    }
    for ( PtsInterval i : intervals )
    {
        wxRect r(makeRect(i));
        r.Offset(-offset);
        dc.DrawRectangle(r);
    }
}

//////////////////////////////////////////////////////////////////////////
// DRAWING
//////////////////////////////////////////////////////////////////////////

void IntervalsView::refreshInterval(PtsInterval interval)
{
    wxRect r(makeRect(interval));
    r.x -= 1;
    r.width += 2; // enlargement to ensure that the vertical black end line of adjacent rects will be (re)drawn. Typical use: remove in the middle of an interval.
    getTimeline().repaint(r);
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

wxRect IntervalsView::makeRect(PtsInterval interval) const
{
    PixelInterval pixels( getZoom().ptsToPixels(interval.lower()), getZoom().ptsToPixels(interval.upper()) );
    return wxRect(pixels.lower(),0,pixels.upper() - pixels.lower() + 1,getSequenceView().getSize().GetHeight());
}

}} // namespace