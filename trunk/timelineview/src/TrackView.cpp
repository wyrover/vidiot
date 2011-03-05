#include "TrackView.h"

#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <boost/foreach.hpp>
#include <algorithm>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/list.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <wx/pen.h>
#include "Layout.h"
#include "Zoom.h"
#include "ClipView.h"
#include "UtilLog.h"
#include "AProjectViewNode.h"
#include "UtilLogStl.h"
#include "Drag.h"
#include "Clip.h"
#include "Selection.h"
#include "Track.h"
#include "ViewMap.h"
#include "PositionInfo.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

TrackView::TrackView(model::TrackPtr track, View* parent)
:   View(parent)
,   mTrack(track)
{
    VAR_DEBUG(this);
    ASSERT(mTrack); // Must be initialized

    getViewMap().registerView(mTrack,this);

    // Not via onClipsAdded: do not trigger a whole sequence of 
    // invalidateBitmaps calls: Bad performance and crashes
    // (view of second item added is not initialized when processing
    // the invalidateBitmap events for the first added item)
    BOOST_FOREACH( model::ClipPtr clip, mTrack->getClips() )
    {
        new ClipView(clip,this);
    }

    mTrack->Bind(model::EVENT_ADD_CLIPS,        &TrackView::onClipsAdded,       this);
    mTrack->Bind(model::EVENT_REMOVE_CLIPS,     &TrackView::onClipsRemoved,     this);
    mTrack->Bind(model::EVENT_HEIGHT_CHANGED,   &TrackView::onHeightChanged,    this);
}

TrackView::~TrackView()
{
    VAR_DEBUG(this);

    mTrack->Unbind(model::EVENT_ADD_CLIPS,      &TrackView::onClipsAdded,       this);
    mTrack->Unbind(model::EVENT_REMOVE_CLIPS,   &TrackView::onClipsRemoved,     this);
    mTrack->Unbind(model::EVENT_HEIGHT_CHANGED, &TrackView::onHeightChanged,    this);

    getViewMap().unregisterView(mTrack);

    BOOST_FOREACH( model::ClipPtr clip, mTrack->getClips() )
    {
        delete getViewMap().getView(clip);
    }
}

model::TrackPtr TrackView::getTrack() const
{
    return mTrack;
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// MODEL EVENTS
//////////////////////////////////////////////////////////////////////////

void TrackView::onClipsAdded( model::EventAddClips& event )
{
    BOOST_FOREACH( model::ClipPtr clip, event.getValue().addClips )
    {
        new ClipView(clip,this);
    }
    invalidateBitmap();
    event.Skip();
}

void TrackView::onClipsRemoved( model::EventRemoveClips& event )
{
    BOOST_FOREACH( model::ClipPtr clip, event.getValue().removeClips )
    {
        delete getViewMap().getView(clip);
    }
    invalidateBitmap();
    event.Skip();
}

void TrackView::onHeightChanged( model::EventHeightChanged& event )
{
    invalidateBitmap();
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// DRAWING EVENTS
//////////////////////////////////////////////////////////////////////////

pixel TrackView::requiredWidth() const
{
    return getParent().requiredWidth();
}

pixel TrackView::requiredHeight() const
{
    return mTrack->getHeight();
}

void TrackView::getPositionInfo(wxPoint position, PointerPositionInfo& info) const
{
    info.clip = mTrack->getClip(getZoom().pixelsToPts(position.x));
    if (info.clip)
    {
        getViewMap().getView(info.clip)->getPositionInfo(position, info);
    }
}

void TrackView::draw(wxBitmap& bitmap) const
{
    wxMemoryDC dc(bitmap);
    wxBrush b(*wxWHITE);
    dc.SetBrush(Layout::sBackgroundBrush);
    dc.SetBrush(b);
    dc.SetPen(Layout::sBackgroundPen);
    dc.DrawRectangle(0,0,bitmap.GetWidth(),bitmap.GetHeight());
    wxPoint position(0,0);    
    BOOST_FOREACH( model::ClipPtr modelclip, mTrack->getClips() )
    {
        wxBitmap bitmap = getViewMap().getView(modelclip)->getBitmap();
        dc.DrawBitmap(bitmap,position);
        position.x += bitmap.GetWidth();
    }
}

void TrackView::drawForDragging(wxPoint position, int height, wxDC& dc, wxDC& dcMask) const
{
    wxPoint pos(position);
    BOOST_FOREACH( model::ClipPtr modelclip, mTrack->getClips() )
    {
        ClipView* view = getViewMap().getView(modelclip);
        view->drawForDragging(pos, height, dc, dcMask);
        pos.x += view->requiredWidth();
    }
}

}} // namespace

