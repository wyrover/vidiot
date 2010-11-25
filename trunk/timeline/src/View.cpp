#include "View.h"

#include <wx/dc.h>
#include <boost/foreach.hpp>
#include "Constants.h"
#include "UtilLog.h"
#include "Cursor.h"
#include "Track.h"
#include "TrackView.h"
#include "Zoom.h"
#include "Sequence.h"
#include "Timeline.h"
#include "MousePointer.h"
#include "ViewMap.h"

namespace gui { namespace timeline {

DEFINE_EVENT(VIEW_UPDATE_EVENT, ViewUpdateEvent, ViewUpdate);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

View::View(IView* parent)
:   mBitmapValid(false)
{
    ASSERT(parent);
    Bind(VIEW_UPDATE_EVENT, &IView::onViewUpdated, parent);
}

View::~View()
{
}

//////////////////////////////////////////////////////////////////////////
// PROPAGATE UPDATES UPWARD
//////////////////////////////////////////////////////////////////////////

void View::onViewUpdated( ViewUpdateEvent& event )
{
    LOG_INFO;
    invalidateBitmap();
}

//////////////////////////////////////////////////////////////////////////
// BITMAP
//////////////////////////////////////////////////////////////////////////

const wxBitmap& View::getBitmap()
{
    if (!mBitmapValid)
    {
        mBitmap.Create(requiredWidth(),requiredHeight());
        draw(mBitmap);
        mBitmapValid = true;
    }
    ASSERT(mBitmapValid && mBitmap.IsOk());
    return mBitmap;
}

void View::invalidateBitmap()
{
    mBitmapValid = false;
    QueueEvent(new ViewUpdateEvent(ViewUpdate(*this,wxRegion())));
}

}} // namespace
