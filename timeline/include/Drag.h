#ifndef DRAG_H
#define DRAG_H

#include <wx/bitmap.h>
#include <wx/dcmemory.h>
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

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Drag(Timeline* timeline);
	
    //////////////////////////////////////////////////////////////////////////
    // START/STOP
    //////////////////////////////////////////////////////////////////////////

    void Start(wxPoint hotspot);
    void MoveTo(wxPoint position);
    void Stop();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////
    
	virtual wxRect GetImageRect(const wxPoint& pos) const;
    wxBitmap getDragBitmap();
    bool isActive() const; ///< @return true if a drag operation with the currently selected clips is in effect.

    //////////////////////////////////////////////////////////////////////////
    // DRAW
    //////////////////////////////////////////////////////////////////////////

    virtual bool DoDrawImage(wxDC& dc, const wxPoint& pos) const;
    void draw(wxDC& dc);
    virtual bool UpdateBackingFromWindow(wxDC& windowDC, wxMemoryDC &destDC, const wxRect& sourceRect, const wxRect &destRect) const;

private:
    wxPoint mHotspot;
    wxBitmap mBitmap;
    bool mActive;
};

}} // namespace

#endif // DRAG_H