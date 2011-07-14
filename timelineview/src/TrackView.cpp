#include "TrackView.h"

#include <algorithm>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
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
#include "Layout.h"
#include "PositionInfo.h"
#include "Selection.h"
#include "Track.h"
#include "TrackEvent.h"
#include "Transition.h"
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
,   mShiftPosition(0)
,   mShiftLength(0)
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
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

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

pixel TrackView::requiredWidth() const
{
    return getParent().getWidth();
}

pixel TrackView::requiredHeight() const
{
    return mTrack->getHeight();
}

void TrackView::setShift(pts position, pts length)
{
    if (length > 0)
    {
        VAR_DEBUG(length);
    }
    mShiftPosition = position;
    mShiftLength = length;
    invalidateBitmap();
}

void TrackView::getPositionInfo(wxPoint position, PointerPositionInfo& info) const
{
    wxPoint adjustedPosition(position);
    if (position.x >= getZoom().ptsToPixels(mShiftPosition))
    {
        if (position.x >= getZoom().ptsToPixels(mShiftPosition + mShiftLength))
        {
            adjustedPosition.x -= getZoom().ptsToPixels(mShiftLength);
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
    wxPoint position(0,0);
    bool shiftApplied = mShiftLength > 0 ? false : true;
    model::IClipPtr previous;
    BOOST_FOREACH( model::IClipPtr modelclip, mTrack->getClips() )
    {
        if (!shiftApplied && modelclip->getLeftPts() >= mShiftPosition)
        {
            position.x += getZoom().ptsToPixels(mShiftLength);
            shiftApplied = true;
        }

        wxBitmap bitmap = getViewMap().getView(modelclip)->getBitmap();

        //if (!modelclip->isA<model::Transition>())
        //{
            wxPoint positionPts(0,0);
            positionPts.x = getZoom().ptsToPixels(modelclip->getLeftPts());
            dc.DrawBitmap(bitmap,positionPts);
            position.x += bitmap.GetWidth();
        //}
        //if (previous && previous->isA<model::Transition>())
        //{
        //    model::TransitionPtr transition = boost::static_pointer_cast<model::Transition>(previous);
        //    pixel transitionxpos = getZoom().ptsToPixels(transition->getLeftPts());
        //    dc.DrawBitmap(bitmap,wxPoint(transitionxpos, position.y));
        //    // todo refactor, duplication with below
        //}
        //previous = modelclip;
    }
    //if (previous && previous->isA<model::Transition>())
    //{
    //    model::TransitionPtr transition = boost::static_pointer_cast<model::Transition>(previous);
    //    pixel transitionxpos = getZoom().ptsToPixels(transition->getLeftPts());
    //    dc.DrawBitmap(bitmap,wxPoint(transitionxpos, position.y));
    //    // todo refactor, duplication with above
    //}
}

void TrackView::drawForDragging(wxPoint position, int height, wxDC& dc, wxDC& dcMask) const
{
    wxPoint pos(position);
    BOOST_FOREACH( model::IClipPtr modelclip, mTrack->getClips() )
    {
        ClipView* view = getViewMap().getView(modelclip);
        // todo first do the clip to the right of the transition
        view->drawForDragging(pos, height, dc, dcMask);
        pos.x += view->getWidth();
    }
}

}} // namespace

