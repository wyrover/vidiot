#include "Scrolling.h"

#include "Timeline.h"

#include "UtilLog.h"
#include "Zoom.h"

namespace gui { namespace timeline {

DEFINE_EVENT(SCROLL_CHANGE_EVENT, ScrollChangeEvent, pts);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Scrolling::Scrolling(Timeline* timeline)
:   wxEvtHandler()
,   Part(timeline)
{
    VAR_DEBUG(this);

    getTimeline().SetScrollRate( 1, 1 );
    getTimeline().EnableScrolling(true,true);
}

Scrolling::~Scrolling()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

wxPoint Scrolling::getOffset() const
{
    int scrollX, scrollY, ppuX, ppuY;
    getTimeline().GetViewStart(&scrollX,&scrollY);
    getTimeline().GetScrollPixelsPerUnit(&ppuX,&ppuY);
    ASSERT_EQUALS(ppuX,1); // Other values not allowed: see Scrolling::align()
    ASSERT_EQUALS(ppuY,1); // Other values not allowed: see Scrolling::align()
    return wxPoint(scrollX * ppuX, scrollY * ppuY);
}

pixel Scrolling::align(pts position, pixel physicalPosition)
{
    pixel diff = ptsToPixel(position) - physicalPosition;
    pixel remaining = 0;
    if (diff != 0)
    {
        int xOld;
        int y;
        getTimeline().GetViewStart(&xOld,&y);
        int xNew = xOld + diff;
        if (xNew < 0)
        {
            remaining = xNew; // return the remaining 'to be scrolled'
            xNew = 0; // Scroll as far as possible
        }
        if (xNew != xOld)
        {
            getTimeline().Scroll(xNew, -1);
        }

    }
    return remaining;
}

pixel Scrolling::ptsToPixel(pts position) const
{
    return getTimeline().CalcScrolledPosition(wxPoint(getZoom().ptsToPixels(position),0)).x;
}

wxPoint Scrolling::getVirtualPosition(wxPoint position) const
{
    wxPoint p;
    getTimeline().CalcUnscrolledPosition(position.x,position.y,&p.x,&p.y);
    return p;
}

}} // namespace