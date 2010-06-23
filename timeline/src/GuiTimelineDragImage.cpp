#include "GuiTimeLineDragImage.h"

#include <wx/dcclient.h>
#include <wx/pen.h>
#include "GuiTimeLine.h"

GuiTimeLineDragImage::GuiTimeLineDragImage(GuiTimeLine* timeline, wxPoint position)
:	wxDragImage(wxCursor(wxCURSOR_HAND))
,   mTimeLine(timeline)
,   mHotspot(position)
,   mBitmap(timeline->getDragBitmap(mHotspot))
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