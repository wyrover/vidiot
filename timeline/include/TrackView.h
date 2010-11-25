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
#include "View.h"

namespace gui { namespace timeline {

class TrackView
    :   public View
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    TrackView(model::TrackPtr track, View* parent);

    void init();

	virtual ~TrackView();

    model::TrackPtr getTrack() const;

     void drawClips(wxPoint position, wxDC& dc, boost::optional<wxDC&> dcMask = boost::none, bool drawDraggedOnly = false);

     //////////////////////////////////////////////////////////////////////////
     // GET/SET
     //////////////////////////////////////////////////////////////////////////

     int requiredWidth();
     int requiredHeight();

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // MODEL EVENTS
    //////////////////////////////////////////////////////////////////////////

    void OnClipsAdded( model::EventAddClips& event );
    void OnClipsRemoved( model::EventRemoveClips& event );

private:

    model::TrackPtr mTrack;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

        /**
    * Overridden from View()
    **/
    void draw(wxBitmap& bitmap);
};

}} // namespace

#endif // TRACK_VIEW_H
