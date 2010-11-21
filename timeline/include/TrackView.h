#ifndef TRACK_VIEW_H
#define TRACK_VIEW_H

#include <wx/bitmap.h>
#include <wx/event.h>
#include <wx/dcmemory.h>
#include <wx/window.h>
#include <boost/tuple/tuple.hpp>
#include <boost/optional.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/version.hpp>
#include "ModelPtr.h"
#include "GuiPtr.h"
#include "UtilEvent.h"
#include "Part.h"

namespace gui { namespace timeline {

class ClipUpdateEvent;

DECLARE_EVENT(TRACK_UPDATE_EVENT, TrackUpdateEvent, TrackView*);

class TrackView
    :   public wxWindow
    ,   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    TrackView(model::TrackPtr track);

    void init();

	virtual ~TrackView();

    int getClipHeight() const;

    model::TrackPtr getTrack() const;

    // todo hide this as the getheight should not be used on the bitmap but on the track
    // for initialization purposes.
    const wxBitmap& getBitmap();

     void drawClips(wxPoint position, wxDC& dc, boost::optional<wxDC&> dcMask = boost::none, bool drawDraggedOnly = false);

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void OnIdle(wxIdleEvent& event);

    //////////////////////////////////////////////////////////////////////////
    // MODEL EVENTS
    //////////////////////////////////////////////////////////////////////////

    void OnClipsAdded( model::EventAddClips& event );
    void OnClipsRemoved( model::EventRemoveClips& event );

    //////////////////////////////////////////////////////////////////////////
    // DRAWING EVENTS
    //////////////////////////////////////////////////////////////////////////

    void OnClipUpdated( ClipUpdateEvent& event );



private:

    void updateBitmap();
    void updateSize();

    model::TrackPtr mTrack;
    wxBitmap mBitmap;

    bool mRedrawOnIdle;
};

}} // namespace

#endif // TRACK_VIEW_H
