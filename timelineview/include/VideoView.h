#ifndef VIDEO_VIEW_H
#define VIDEO_VIEW_H

#include "View.h"

namespace model {
    class Track;
    typedef boost::shared_ptr<Track> TrackPtr;
    class EventAddVideoTracks;
    class EventRemoveVideoTracks;
}

namespace test {
    class TestTimeline;
}

namespace gui { namespace timeline {

struct PointerPositionInfo;

class VideoView
    :   public View
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    VideoView(View* parent);
    virtual ~VideoView();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    wxSize requiredSize() const override;  ///< @see View::requiredSize()

    void getPositionInfo(wxPoint position, PointerPositionInfo& info) const;

    /// \return y position of the track within this View
    pixel getPosition(model::TrackPtr track) const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MODEL EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onVideoTracksAdded( model::EventAddVideoTracks& event );
    void onVideoTracksRemoved( model::EventRemoveVideoTracks& event );

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void draw(wxBitmap& bitmap) const override; ///< @see View::draw()
};

}} // namespace

#endif // VIDEO_VIEW_H