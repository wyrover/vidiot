#ifndef GUI_TIME_LINE_DRAG_IMAGE_H
#define GUI_TIME_LINE_DRAG_IMAGE_H

#include <wx/image.h>

#include <wx/dcmemory.h>
#include <wx/bitmap.h>
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
	Drag(Timeline* timeline);
	virtual bool DoDrawImage(wxDC& dc, const wxPoint& pos) const;
	virtual wxRect GetImageRect(const wxPoint& pos) const;
    wxBitmap getDragBitmap();// const;
    void Start(wxPoint hotspot);
    void MoveTo(wxPoint position);
    void Stop();
    /** @return true if a drag operation with the currently selected clips is in effect. */
    bool isActive() const;
    virtual bool UpdateBackingFromWindow(wxDC& windowDC, wxMemoryDC &destDC, const wxRect& sourceRect, const wxRect &destRect) const;
    void draw(wxDC& dc);

private:
    wxPoint mHotspot;
    wxBitmap mBitmap;
    bool mActive;
};

}} // namespace

#endif // GUI_TIME_LINE_DRAG_IMAGE_H