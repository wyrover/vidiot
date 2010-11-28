#include "View.h"
#include "UtilLog.h"
#include "Timeline.h"

namespace gui { namespace timeline {

DEFINE_EVENT(VIEW_UPDATE_EVENT, ViewUpdateEvent, ViewUpdate);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

View::View(Timeline* timeline)
:   wxEvtHandler()
,   Part(timeline)
,   mBitmapValid(false)
{
    ASSERT(timeline);
    Bind(VIEW_UPDATE_EVENT, &Timeline::onViewUpdated, timeline);
}

View::View(Part* parent)
:   wxEvtHandler()
,   Part(&(parent->getTimeline()))
,   mBitmapValid(false)
{
    ASSERT(parent);
    Bind(VIEW_UPDATE_EVENT, &Part::onViewUpdated, parent);
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
