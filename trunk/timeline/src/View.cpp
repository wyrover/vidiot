#include "View.h"
#include "UtilLog.h"
#include "Timeline.h"

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
    ASSERT(timeline);
    //mEvtHandler.Bind(VIEW_UPDATE_EVENT, &View::onViewUpdated, timeline);
}

View::View(View* parent)
:   Part(&(parent->getTimeline()))
,   mEvtHandler()
,   mParent(parent)
,   mBitmapValid(false)
{
    ASSERT(parent);
    mEvtHandler.Bind(VIEW_UPDATE_EVENT, &View::onChildViewUpdated, parent);
}

View::~View()
{
    mEvtHandler.Unbind(VIEW_UPDATE_EVENT, &View::onChildViewUpdated, mParent);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

View& View::getParent() const
{
    return *mParent;
}

//////////////////////////////////////////////////////////////////////////
// PROPAGATE UPDATES UPWARD
//////////////////////////////////////////////////////////////////////////

void View::onChildViewUpdated( ViewUpdateEvent& event )
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
    mBitmapValid = false;
    mEvtHandler.QueueEvent(new ViewUpdateEvent(ViewUpdate(*this,wxRegion())));
}

}} // namespace
