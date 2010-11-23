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

View::View()
:   mParent(0)
{
}

View::View(View* parent)
:   mParent(parent)
{
    Bind(VIEW_UPDATE_EVENT, &View::onViewUpdated, parent);
}

void View::init()
{
    if (!mParent)
    {
        Bind(VIEW_UPDATE_EVENT, &Timeline::onViewUpdated, &getTimeline());
    }
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
    }
    return mBitmap;
}

void View::invalidateBitmap()
{
    mBitmapValid = false;
    QueueEvent(new ViewUpdateEvent(ViewUpdate(*this,wxRegion())));
}

}} // namespace
