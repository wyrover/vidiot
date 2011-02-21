#include "View.h"
#include "UtilLog.h"
#include "Timeline.h"
#include "Zoom.h"

namespace gui { namespace timeline {

DEFINE_EVENT(VIEW_UPDATE_EVENT, ViewUpdateEvent, ViewUpdate);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

View::View(Timeline* timeline)
:   Part(timeline)
,   mEvtHandler()
,   mParent(0)
,   mBitmapValid(false)
{
    VAR_DEBUG(this);
    ASSERT(timeline);
}

void View::init()
{
    VAR_DEBUG(this);
    ASSERT(this == &getTimeline());
    mEvtHandler.Bind(VIEW_UPDATE_EVENT, &Timeline::onViewUpdated, &getTimeline());
    getZoom().Bind(ZOOM_CHANGE_EVENT, &Timeline::onZoomChanged, &getTimeline());
}

void View::deinit()
{
    VAR_DEBUG(this);
    ASSERT(this == &getTimeline());
    mEvtHandler.Unbind(VIEW_UPDATE_EVENT, &Timeline::onViewUpdated, &getTimeline());
    getZoom().Unbind(ZOOM_CHANGE_EVENT, &Timeline::onZoomChanged, &getTimeline());
}

View::View(View* parent)
:   Part(&(parent->getTimeline()))
,   mEvtHandler()
,   mParent(parent)
,   mBitmapValid(false)
{
    VAR_DEBUG(this);
    ASSERT(mParent);
    mEvtHandler.Bind(VIEW_UPDATE_EVENT, &View::onChildViewUpdated, mParent);
    getZoom().Bind(ZOOM_CHANGE_EVENT, &View::onZoomChanged, this);
}

View::~View()
{
    if (mParent)
    {
        mEvtHandler.Unbind(VIEW_UPDATE_EVENT, &View::onChildViewUpdated, mParent);
        getZoom().Unbind(ZOOM_CHANGE_EVENT, &View::onZoomChanged, this);

    }
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

View& View::getParent() const
{
    return *mParent;
}

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

void View::onChildViewUpdated( ViewUpdateEvent& event )
{
    invalidateBitmap();
    event.Skip();
}

void View::onZoomChanged( ZoomChangeEvent& event )
{
    invalidateBitmap();
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// BITMAP
//////////////////////////////////////////////////////////////////////////

const wxBitmap& View::getBitmap() const
{
    if (!mBitmapValid)
    {
        int w = requiredWidth();
        int h = requiredHeight();
        if ((mBitmap.GetWidth() != w) || (mBitmap.GetHeight() != h))
        {
            mBitmap.Create(requiredWidth(),requiredHeight());
        }
        draw(mBitmap);
        mBitmapValid = true;
    }
    ASSERT(mBitmapValid && mBitmap.IsOk());
    return mBitmap;
}

void View::invalidateBitmap()
{
    VAR_DEBUG(this);
    mBitmapValid = false;
    mEvtHandler.QueueEvent(new ViewUpdateEvent(ViewUpdate(*this,wxRegion())));
}

}} // namespace
