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
    ASSERT(ppuX == 1 && ppuY == 1)(ppuX)(ppuY); // Other values not allowed: see Scrolling::align()
    return wxPoint(scrollX * ppuX, scrollY * ppuY);
}

void Scrolling::align(pts position, pixel physicalPosition)
{
    pixel diff = ptsToPixel(position) - physicalPosition;
    if (diff != 0)
    {
        int x;
        int y;
        getTimeline().GetViewStart(&x,&y);
        getTimeline().Scroll(x + diff, -1);
    }
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
