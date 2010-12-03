#ifndef DRAG_H
#define DRAG_H

#include <list>
#include <wx/bitmap.h>
#include <wx/dcmemory.h>
#include "Part.h"

#define wxUSE_GENERIC_DRAGIMAGE 1
#include <wx/generic/dragimgg.h>
#define wxDragImage wxGenericDragImage

namespace model {
    class Track;
    typedef boost::shared_ptr<Track> TrackPtr;
    typedef std::list<TrackPtr> Tracks;
}

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
    void draw(wxDC& dc) const;
    //virtual bool UpdateBackingFromWindow(wxDC& windowDC, wxMemoryDC &destDC, const wxRect& sourceRect, const wxRect &destRect) const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxPoint mHotspot;
    wxPoint mPosition;
    wxBitmap mBitmap;
    bool mActive;
    bool mSnap; ///< true if the drag image snaps to the nearest track(s)

    model::Tracks mDraggedVideo;
    model::Tracks mDraggedAudio;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void initializeTracks(); ///< Initializes the locally stored tracks used for dragging
};

}} // namespace

#endif // DRAG_H