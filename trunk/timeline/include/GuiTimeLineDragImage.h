#ifndef GUI_TIME_LINE_DRAG_IMAGE_H
#define GUI_TIME_LINE_DRAG_IMAGE_H

#include <wx/image.h>

#include <wx/dcmemory.h>
#include "GuiPtr.h"

#define wxUSE_GENERIC_DRAGIMAGE 1
#include <wx/generic/dragimgg.h>
#define wxDragImage wxGenericDragImage

namespace gui { namespace timeline {

class GuiTimeLineDragImage : public wxDragImage
{
public:
	GuiTimeLineDragImage(GuiTimeLine* timeline, wxPoint position);
	virtual bool DoDrawImage(wxDC& dc, const wxPoint& pos) const;
	virtual wxRect GetImageRect(const wxPoint& pos) const;
    wxPoint getHotspot() const { return mHotspot; };
private:
    GuiTimeLine* mTimeLine;
    wxPoint mHotspot;
    wxBitmap mBitmap;
};

}} // namespace

#endif // GUI_TIME_LINE_DRAG_IMAGE_H