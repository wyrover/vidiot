#include "Scrolling.h"

#include "UtilLog.h"
#include "Timeline.h"
#include "Zoom.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Scrolling::Scrolling(Timeline* timeline)
:   Part(timeline)
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

void Scrolling::align(pts sequence, pts pointer)
{
    VAR_DEBUG(sequence)(pointer);

    pts diff =  sequence - pointer;
    if (diff != 0)
    {
        int x;
        int y;
        getTimeline().GetViewStart(&x,&y);
        getTimeline().Scroll(x + getZoom().ptsToPixels(diff), y);
        getTimeline().Refresh();
        getTimeline().Update();
    }
}


}} // namespace
