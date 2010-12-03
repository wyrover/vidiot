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
#include "Constants.h"
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
    ASSERT(mTrack); // Must be initialized
    getViewMap().registerView(mTrack,this);

    model::MoveParameter m;
    m.addClips = mTrack->getClips();
    OnClipsAdded(model::EventAddClips(m));

    mTrack->Bind(model::EVENT_ADD_CLIPS,     &TrackView::OnClipsAdded,    this);
    mTrack->Bind(model::EVENT_REMOVE_CLIPS,  &TrackView::OnClipsRemoved,  this);
}

TrackView::~TrackView()
{
    getViewMap().unregisterView(mTrack);
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

void TrackView::OnClipsAdded( model::EventAddClips& event )
{
    BOOST_FOREACH( model::ClipPtr clip, event.getValue().addClips )
    {
        ClipView* p = new ClipView(clip,this);
    }
    invalidateBitmap();
}

void TrackView::OnClipsRemoved( model::EventRemoveClips& event )
{
    BOOST_FOREACH( model::ClipPtr clip, event.getValue().removeClips )
    {
// @todo cleanup:        getViewMap().getView(clip)->Destroy();
    }
    invalidateBitmap();
}

//////////////////////////////////////////////////////////////////////////
// DRAWING EVENTS
//////////////////////////////////////////////////////////////////////////

int TrackView::requiredWidth() const
{
    return getParent().requiredWidth();
}

int TrackView::requiredHeight() const
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
    dc.SetBrush(Constants::sBackgroundBrush);
    dc.SetBrush(b);
    dc.SetPen(Constants::sBackgroundPen);
    dc.DrawRectangle(0,0,bitmap.GetWidth(),bitmap.GetHeight());
    wxPoint position(0,0);    
    drawClips(position, dc);
}

void TrackView::drawClips(wxPoint position, wxDC& dc, boost::optional<wxDC&> dcMask, bool drawDraggedOnly) const
{
    // if dcMask holds, then we're drawing a 'drag image'. Otherwise, we're drawing the regular track bitmap.
    bool draggedClipsOnly = dcMask;

    wxPoint pos(position);
    BOOST_FOREACH( model::ClipPtr modelclip, mTrack->getClips() )
    {
        wxBitmap bitmap = getViewMap().getView(modelclip)->getBitmap();
        bool isBeingDragged = getDrag().isActive() && getSelection().isSelected(modelclip);

        if (draggedClipsOnly)
        {
            // Drawing the dragged clips
            if (isBeingDragged)
            {
                dc.DrawBitmap(bitmap,pos);
                dcMask->DrawRectangle(pos,bitmap.GetSize());
            }
        }
        else
        {
            // Regular track drawing
            if (!isBeingDragged)
            {
                dc.DrawBitmap(bitmap,pos);
            }
            else
            {
                dc.SetBrush(Constants::sBackgroundBrush);
                dc.SetPen(Constants::sBackgroundPen);
                dc.DrawRectangle(pos,bitmap.GetSize());
            }
        }
        pos.x += bitmap.GetWidth();
    }
}

}} // namespace

