#ifndef TRACK_VIEW_H
#define TRACK_VIEW_H

#include "View.h"
#include <boost/optional.hpp>
#include <ostream>

namespace model {
class Track;
typedef boost::shared_ptr<Track> TrackPtr;
class EventAddClips;
class EventRemoveClips;
class EventHeightChanged;
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

    /// Draw the selected clips of this track only.
    /// This is required for drawing these clips when dragging.
    /// The height parameter is required for the case where this
    /// track is dragged over another track which has another height.
    void drawForDragging(wxPoint position, int height, wxDC& dc, wxDC& dcMask) const;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    wxSize requiredSize() const override;  ///< @see View::requiredSize()

    void getPositionInfo(wxPoint position, PointerPositionInfo& info) const;

    void onShiftChanged();

    //////////////////////////////////////////////////////////////////////////
    // MODEL EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onClipsAdded( model::EventAddClips& event );
    void onClipsRemoved( model::EventRemoveClips& event );
    void onHeightChanged( model::EventHeightChanged& event );

private:

    model::TrackPtr mTrack;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void draw(wxBitmap& bitmap) const override; ///< @see View::draw()
};

}} // namespace

#endif // TRACK_VIEW_H