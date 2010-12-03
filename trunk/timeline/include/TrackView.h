#ifndef TRACK_VIEW_H
#define TRACK_VIEW_H

#include "View.h"
#include <boost/optional.hpp>

namespace model {
    class Track;
    typedef boost::shared_ptr<Track> TrackPtr;
    class EventAddClips;
    class EventRemoveClips;
}

namespace gui { namespace timeline {
    struct PointerPositionInfo;

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

     void drawClips(wxPoint position, wxDC& dc, boost::optional<wxDC&> dcMask = boost::none, bool drawDraggedOnly = false) const;

     //////////////////////////////////////////////////////////////////////////
     // GET/SET
     //////////////////////////////////////////////////////////////////////////

     int requiredWidth() const;  ///< @see View::requiredWidth()
     int requiredHeight() const; ///< @see View::requiredHeight()

     void getPositionInfo(wxPoint position, PointerPositionInfo& info) const;

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

    void draw(wxBitmap& bitmap) const; ///< @see View::draw()
};

}} // namespace

#endif // TRACK_VIEW_H
