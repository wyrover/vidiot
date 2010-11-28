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
	virtual ~TrackView();

    model::TrackPtr getTrack() const;

     void drawClips(wxPoint position, wxDC& dc, boost::optional<wxDC&> dcMask = boost::none, bool drawDraggedOnly = false);

     //////////////////////////////////////////////////////////////////////////
     // GET/SET
     //////////////////////////////////////////////////////////////////////////

     /** @see View::requiredWidth() **/
     int requiredWidth();

     /** @see View::requiredHeight() **/
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

    /** @see View::draw() */
    void draw(wxBitmap& bitmap);
};

}} // namespace

#endif // TRACK_VIEW_H
