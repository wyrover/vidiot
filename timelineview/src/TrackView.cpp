#include "TrackView.h"

#include <algorithm>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <wx/dcclient.h>
#include <wx/dcmemory.h>
#include <wx/pen.h>
#include "Node.h"
#include "Clip.h"
#include "ClipView.h"
#include "Drag.h"
#include "Drag_Shift.h"
#include "Layout.h"
#include "PositionInfo.h"
#include "Selection.h"
#include "Track.h"
#include "TrackEvent.h"
#include "Transition.h"
#include "UtilLogWxwidgets.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "ViewMap.h"
#include "Zoom.h"

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
    BOOST_FOREACH( model::IClipPtr clip, mTrack->getClips() )
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

    BOOST_FOREACH( model::IClipPtr clip, mTrack->getClips() )
    {
        delete getViewMap().getView(clip);
    }
}

model::TrackPtr TrackView::getTrack() const
{
    return mTrack;
}

//////////////////////////////////////////////////////////////////////////
// MODEL EVENTS
//////////////////////////////////////////////////////////////////////////

void TrackView::onClipsAdded( model::EventAddClips& event )
{
    BOOST_FOREACH( model::IClipPtr clip, event.getValue().addClips )
    {
        new ClipView(clip,this);
    }
    invalidateBitmap();
    event.Skip();
}

void TrackView::onClipsRemoved( model::EventRemoveClips& event )
{
    BOOST_FOREACH( model::IClipPtr clip, event.getValue().removeClips )
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

wxSize TrackView::requiredSize() const
{
    return wxSize(getParent().getSize().GetWidth(),mTrack->getHeight());
}

void TrackView::onShiftChanged()
{
    invalidateBitmap();
}

void TrackView::getPositionInfo(wxPoint position, PointerPositionInfo& info) const
{
    wxPoint adjustedPosition(position);

    Shift shift = getDrag().getShift();
    if (shift && (position.x >= getZoom().ptsToPixels(shift->mPosition)))
    {
        // Apply shift if (A) shift is enabled, and (B) current position is after the shift start
        if (position.x >= getZoom().ptsToPixels(shift->mPosition + shift->mLength))
        {
            // Clip is AFTER the shifted area, adjust accordingly
            adjustedPosition.x -= getZoom().ptsToPixels(shift->mLength);
        }
        else
        {
            return; // Inside shifted area. No clip there.
        }
    }

    info.clip = mTrack->getClip(getZoom().pixelsToPts(adjustedPosition.x));
    if (info.clip)
    {
        getViewMap().getView(info.clip)->getPositionInfo(adjustedPosition, info);
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
    std::list<bool> tf = boost::assign::list_of(false)(true);
    BOOST_FOREACH( bool transitionValue, tf ) // First, normal clips, second transitions
    {
        BOOST_FOREACH( model::IClipPtr modelclip, mTrack->getClips() )
        {
            ClipView* view = getViewMap().getView(modelclip);
            wxBitmap bitmap = view->getBitmap();
            if (modelclip->isA<model::Transition>() == transitionValue)
            {
                pts left = view->getLeftPts();
                wxPoint position(getZoom().ptsToPixels(left),0);
                Shift shift = getDrag().getShift();
                if (shift && left >= shift->mPosition)
                {
                    position.x += getZoom().ptsToPixels(shift->mLength);
                }
                dc.DrawBitmap(bitmap,position);
            }
        }
    }
}

void TrackView::drawForDragging(wxPoint position, int height, wxDC& dc, wxDC& dcMask) const
{
    std::list<bool> tf = boost::assign::list_of(false)(true);
    BOOST_FOREACH( bool transitionValue, tf ) // First, normal clips, second transitions
    {
        BOOST_FOREACH( model::IClipPtr modelclip, mTrack->getClips() )
        {
            if (modelclip->isA<model::Transition>() == transitionValue)
            {
                ClipView* view = getViewMap().getView(modelclip);
                pixel left = view->getLeftPixel();
                view->drawForDragging(wxPoint(position.x + left,position.y), height, dc, dcMask);
            }
        }
    }
}
}} // namespace