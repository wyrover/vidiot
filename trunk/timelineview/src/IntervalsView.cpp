#include "IntervalsView.h"

#include <boost/foreach.hpp>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
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

    wxBitmap bmp(2,2);
    wxMemoryDC dcM(bmp);
    dcM.SelectObject(wxNullBitmap);

    dc.SetPen(*wxGREY_PEN);
    wxBrush b(*wxLIGHT_GREY,wxBRUSHSTYLE_CROSSDIAG_HATCH);
    dc.SetBrush(b);

    BOOST_FOREACH( PtsInterval i, intervals )
    {
        dc.DrawRectangle(makeRect(i));
    }
}

void IntervalsView::refreshInterval(PtsInterval interval)
{
    wxRect r(makeRect(interval));

    invalidateBitmap();

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