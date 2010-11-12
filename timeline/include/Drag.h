#ifndef GUI_TIME_LINE_DRAG_IMAGE_H
#define GUI_TIME_LINE_DRAG_IMAGE_H

#include <wx/image.h>

#include <wx/dcmemory.h>
#include <wx/bitmap.h>
#include "GuiPtr.h"
#include "Part.h"

#define wxUSE_GENERIC_DRAGIMAGE 1
#include <wx/generic/dragimgg.h>
#define wxDragImage wxGenericDragImage

namespace gui { namespace timeline {

class Drag
    :   public Part
    ,   public wxDragImage
{
public:
	Drag();
	virtual bool DoDrawImage(wxDC& dc, const wxPoint& pos) const;
	virtual wxRect GetImageRect(const wxPoint& pos) const;
    void setHotspot(wxPoint position);
    wxPoint getHotspot() const { return mHotspot; };
    wxBitmap getDragBitmap();// const;
    void Start(wxPoint hotspot);
    void MoveTo(wxPoint hotspot);
    void Stop();

private:
    wxPoint mHotspot;
    wxBitmap mBitmap;
};

}} // namespace

#endif // GUI_TIME_LINE_DRAG_IMAGE_H