#include "GuiTimeLineDragImage.h"

#include <wx/dcclient.h>
#include <wx/pen.h>
#include <boost/foreach.hpp>
#include "GuiTimeLine.h"
#include "Constants.h"
#include "GuiTimeLineTrack.h"
#include "Track.h"
#include "Sequence.h"

namespace gui { namespace timeline {

GuiTimeLineDragImage::GuiTimeLineDragImage(GuiTimeLine& timeline, wxPoint position)
:	wxDragImage(wxCursor(wxCURSOR_HAND))
,   mTimeLine(timeline)
,   mHotspot(position)
,   mBitmap(getDragBitmap())
{
}

bool GuiTimeLineDragImage::DoDrawImage(wxDC& dc, const wxPoint& pos) const
{
    dc.DrawBitmap(mBitmap, pos, true);
	return true;
}

wxRect GuiTimeLineDragImage::GetImageRect(const wxPoint& pos) const
{
	return wxRect(pos.x,pos.y,mBitmap.GetWidth(),mBitmap.GetHeight());
}

wxBitmap GuiTimeLineDragImage::getDragBitmap() //const
{
    int w = mTimeLine.getWidth();
    int h = mTimeLine.getHeight();

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
    wxPoint position(0,mTimeLine.getDividerPosition());
    BOOST_REVERSE_FOREACH( model::TrackPtr track, mTimeLine.getSequence()->getVideoTracks() )
    {
        position.y -= track->getHeight();
    }

    // Draw video tracks
    BOOST_REVERSE_FOREACH( model::TrackPtr track, mTimeLine.getSequence()->getVideoTracks() )
    {
        mTimeLine.getViewMap().getView(track)->drawClips(position,dc,dcMask);
        position.y += track->getHeight();
    }

    // Draw audio tracks
    position.y += Constants::sAudioVideoDividerHeight;
    BOOST_FOREACH( model::TrackPtr track, mTimeLine.getSequence()->getAudioTracks() )
    {
        mTimeLine.getViewMap().getView(track)->drawClips(position,dc,dcMask);
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

    return temp.GetSubBitmap(wxRect(origin_x,origin_y,size_x,size_y));
}
}} // namespace
