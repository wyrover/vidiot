#include "Drag.h"

#include <wx/pen.h>
#include <boost/foreach.hpp>
#include "Timeline.h"
#include "Layout.h"
#include "TrackView.h"
#include "Track.h"
#include "Sequence.h"
#include "Selection.h"
#include "ViewMap.h"
#include "Divider.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Drag::Drag(Timeline* timeline)
    :	wxDragImage(wxCursor(wxCURSOR_HAND))
    ,   Part(timeline)
    ,   mHotspot(0,0)
    ,   mPosition(0,0)
    ,   mBitmap()
    ,   mActive(false)
    ,   mSequence()
{
}

//////////////////////////////////////////////////////////////////////////
// START/STOP
//////////////////////////////////////////////////////////////////////////

void Drag::Start(wxPoint hotspot)
{
    mHotspot = hotspot;
    mPosition = hotspot;
    mActive = true; // Must be done BEFORE getDragBitmap(), since it is used for creating that bitmap.
    getSelection().invalidateTracksWithSelectedClips();
    //getTimeline().Refresh();
    mBitmap = getDragBitmap();
    //bool ok = BeginDrag(mHotspot, &getTimeline(), false);
    //ASSERT(ok);
    //Show();
    MoveTo(hotspot);
}

void Drag::MoveTo(wxPoint position)
{
    //Move(position - getTimeline().getScrollOffset());

    getTimeline().invalidateBitmap();
    wxRegion redrawRegion(wxRect(mPosition - mHotspot, mBitmap.GetSize()));
    mPosition = position;
    redrawRegion.Union(wxRect(mPosition - mHotspot, mBitmap.GetSize()));
    wxRegionIterator it(redrawRegion);
    while (it)
    {
        getTimeline().RefreshRect(it.GetRect());
        it++;
    }

//    getTimeline().RefreshRect(wxRect(mPosition - mHotspot, mBitmap.GetSize()));
}

void Drag::Stop()
{
    mActive = false;
    getSelection().invalidateTracksWithSelectedClips();
    //Hide();
    //EndDrag();
    getTimeline().Refresh();
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

wxRect Drag::GetImageRect(const wxPoint& pos) const
{
    return wxRect(pos.x,pos.y,mBitmap.GetWidth(),mBitmap.GetHeight());
}

wxBitmap Drag::getDragBitmap() //const
{
    int w = getTimeline().requiredWidth();
    int h = getTimeline().requiredHeight();

    wxBitmap temp(w,h); // Create a bitmap equal in size to the entire virtual area (for simpler drawing code)
    wxBitmap mask(w,h,1);

    wxMemoryDC dc(temp); // Must go out of scope to be able to use temp.data below
    wxMemoryDC dcMask(mask);

    dc.SetBackground(*wxTRANSPARENT_BRUSH);
    dc.Clear();
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(0,0,w,h);

    dcMask.SetBackground(*wxBLACK_BRUSH);
    dcMask.Clear();
    dcMask.SetPen(*wxWHITE_PEN);
    dcMask.SetBrush(*wxWHITE_BRUSH);

    // Draw video tracks
    wxPoint position(0,getDivider().getVideoPosition());
    BOOST_REVERSE_FOREACH( model::TrackPtr track, getSequence()->getVideoTracks() )
    {
        getViewMap().getView(track)->drawClips(position,dc,dcMask);
        position.y += track->getHeight() + Layout::sTrackDividerHeight;
    }

    // Draw audio tracks
    position.y = getDivider().getAudioPosition();
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getAudioTracks() )
    {
        getViewMap().getView(track)->drawClips(position,dc,dcMask);
        position.y += track->getHeight();
    }

    int origin_x = std::max(dcMask.MinX(),0);
    int origin_y = std::max(dcMask.MinY(),0);
    int size_x = dcMask.MaxX() - origin_x;
    int size_y = dcMask.MaxY() - origin_y;

    dc.SelectObject(wxNullBitmap);
    dcMask.SelectObject(wxNullBitmap);

    temp.SetMask(new wxMask(mask));

    mHotspot.x -= origin_x;
    mHotspot.y -= origin_y;

    VAR_DEBUG(origin_x)(origin_y)(size_x)(size_y);
    ASSERT(size_x > 0 && size_y > 0)(size_x)(size_y);
    return temp.GetSubBitmap(wxRect(origin_x,origin_y,size_x,size_y));
}

bool Drag::isActive() const
{
    return mActive;
}

//////////////////////////////////////////////////////////////////////////
// DRAW
//////////////////////////////////////////////////////////////////////////

bool Drag::DoDrawImage(wxDC& dc, const wxPoint& pos) const
{
    //dc.DrawBitmap(mBitmap, pos, true);

    wxBitmap b = mBitmap;
    wxMemoryDC dcBmp(b);
    int x = pos.x;
    int y = pos.y;
    int w = mBitmap.GetWidth();
    int h = mBitmap.GetHeight();
    dc.Blit(x,y,w,h,&dcBmp,x,y,wxCOPY);

    return true;
}

void Drag::draw(wxDC& dc) const
{
    if (!mActive)
    {
        return;
    }
    dc.DrawBitmap(mBitmap,mPosition - mHotspot,true);

        //dc.SetPen(Constants::sDebugPen);
        //dc.SetBrush(Constants::sDebugBrush);
        //dc.DrawRectangle(offset,b.GetSize());


    // UNUSED YET.
    //wxPoint scroll = getTimeline().getScrollOffset();
    //wxRegionIterator upd(getTimeline().GetUpdateRegion()); // get the update rect list
    //while (upd)
    //{
    //    int x = scroll.x + upd.GetX();
    //    int y = scroll.y + upd.GetY();
    //    int w = upd.GetW();
    //    int h = upd.GetH();
    //    VAR_DEBUG(x)(y)(w)(h);
    //    dc.Blit(x,y,w,h,&dcBmp,x,y,wxCOPY,false,0,0);
    //    upd++;
    //}
}

//bool Drag::UpdateBackingFromWindow(wxDC& windowDC, wxMemoryDC &destDC, const wxRect& sourceRect, const wxRect &destRect) const
//{
//    int x = sourceRect.GetX();
//    int y = sourceRect.GetY();
//    int w = sourceRect.GetWidth();
//    int h = sourceRect.GetHeight();
//    destDC.Blit(x,y,w,h,&windowDC,x,y,wxCOPY,false,0,0);
//    return true;
//}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void prune(model::TrackPtr track)
{
    //model::Clips clips = track->getClips();
    BOOST_FOREACH(model::ClipPtr clip, track->getClips())
    {
        
    }
}

void Drag::prepareDrag()
{
    mSequence = make_cloned<model::Sequence>(getSequence());
    BOOST_FOREACH(model::TrackPtr track, mSequence->getVideoTracks())
    {
        prune(track);
    }
    BOOST_FOREACH(model::TrackPtr track, mSequence->getAudioTracks())
    {
        prune(track);
    }
}


}} // namespace
