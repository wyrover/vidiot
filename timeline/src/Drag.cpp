#include "Drag.h"

#include <wx/dcclient.h>
#include <wx/pen.h>
#include <boost/foreach.hpp>
#include "Timeline.h"
#include "Constants.h"
#include "TrackView.h"
#include "Track.h"
#include "Sequence.h"
#include "ViewMap.h"

namespace gui { namespace timeline {

Drag::Drag(Timeline* timeline)
:	wxDragImage(wxCursor(wxCURSOR_HAND))
,   Part(timeline)
,   mHotspot(wxPoint(0,0))
,   mBitmap()
,   mActive(false)
{
}

bool Drag::DoDrawImage(wxDC& dc, const wxPoint& pos) const
{
    //dc.DrawBitmap(mBitmap, pos, true);

    wxBitmap b = mBitmap;
    wxMemoryDC dcBmp(b);
    int x = pos.x;
    int y = pos.y;
    int w = mBitmap.GetWidth();
    int h = mBitmap.GetHeight();
    dc.Blit(x,y,w,h,&dcBmp,x,y,wxCOPY,false,0,0);

	return true;
}

wxRect Drag::GetImageRect(const wxPoint& pos) const
{
	return wxRect(pos.x,pos.y,mBitmap.GetWidth(),mBitmap.GetHeight());
}

wxBitmap Drag::getDragBitmap() //const
{
    int w = getTimeline().getWidth();
    int h = getTimeline().getHeight();

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

    // First determine starting point
    wxPoint position(0,getTimeline().getDividerPosition());
    BOOST_REVERSE_FOREACH( model::TrackPtr track, getSequence()->getVideoTracks() )
    {
        position.y -= track->getHeight() + Constants::sTrackDividerHeight;
    }

    // Draw video tracks
    BOOST_REVERSE_FOREACH( model::TrackPtr track, getSequence()->getVideoTracks() )
    {
        getViewMap().getView(track)->drawClips(position,dc,dcMask);
        position.y += track->getHeight() + Constants::sTrackDividerHeight;
    }

    // Draw audio tracks
    position.y += Constants::sAudioVideoDividerHeight;
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

void Drag::Start(wxPoint hotspot)
{
    mHotspot = hotspot;
    mActive = true; // Must be done BEFORE getDragBitmap(), since it is used for creating that bitmap.
    mBitmap = getDragBitmap();
    bool ok = BeginDrag(mHotspot, &getTimeline(), false);
    ASSERT(ok);
    getTimeline().Refresh(false);
    getTimeline().Update();
    Show();
    MoveTo(hotspot);
}

bool Drag::isActive() const
{
    return mActive;
}

void Drag::Stop()
{
    mActive = false;
    Hide();
    EndDrag();
    getTimeline().Refresh();
}

void Drag::MoveTo(wxPoint position)
{
//    Hide();
    //getTimeline().Refresh(false);
    //getTimeline().Update();
    Move(position - getTimeline().getScrollOffset());
//    Show();
}

bool Drag::UpdateBackingFromWindow(wxDC& windowDC, wxMemoryDC &destDC, const wxRect& sourceRect, const wxRect &destRect) const
{
    int x = sourceRect.GetX();
    int y = sourceRect.GetY();
    int w = sourceRect.GetWidth();
    int h = sourceRect.GetHeight();
    destDC.Blit(x,y,w,h,&windowDC,x,y,wxCOPY,false,0,0);
    return true;
}

//////////////////////////////////////////////////////////////////////////
// DRAWING
//////////////////////////////////////////////////////////////////////////

void Drag::draw(wxDC& dc)
{
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

}} // namespace
