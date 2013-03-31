#include "View.h"

#include "Layout.h"
#include "Timeline.h"
#include "UtilLog.h"
#include "ViewUpdateEvent.h"
#include "Zoom.h"

namespace gui { namespace timeline {

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
    ASSERT_EQUALS(this,&getTimeline());
    mEvtHandler.Bind(VIEW_UPDATE_EVENT, &Timeline::onViewUpdated, &getTimeline());
    getZoom().Bind(ZOOM_CHANGE_EVENT, &Timeline::onZoomChanged, &getTimeline());
}

void View::deinit()
{
    VAR_DEBUG(this);
    ASSERT_EQUALS(this,&getTimeline());
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
        wxSize required = requiredSize();
        if (mBitmap.GetSize() != required)
        {
            mBitmap.Create(required);
        }
        draw(mBitmap);
        mBitmapValid = true;
    }
    ASSERT(mBitmapValid && mBitmap.IsOk());
    return mBitmap;
}

wxSize View::getSize() const
{
    if (!mBitmapValid)
    {
        return requiredSize();
    }
    return mBitmap.GetSize();
}

void View::invalidateBitmap()
{
    VAR_DEBUG(this);
    mBitmapValid = false;
    mEvtHandler.ProcessEvent(ViewUpdateEvent(ViewUpdate(*this,wxRegion())));
}

void View::drawDivider(wxDC& dc, pixel yPosition, pixel height) const
{
    wxRect rect(0, yPosition, dc.GetSize().GetWidth(), height);
    dc.SetPen(Layout::get().DividerPen);
    dc.SetBrush(Layout::get().DividerBrush);
        //*wxLIGHT_GREY_BRUSH);
    dc.DrawRectangle(rect);
}

}} // namespace