#ifndef VIDEO_VIEW_H
#define VIDEO_VIEW_H

#include "View.h"

namespace model {
    class EventAddVideoTracks;
    class EventRemoveVideoTracks;
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

    int requiredWidth() const;  ///< @see View::requiredWidth()
    int requiredHeight() const; ///< @see View::requiredHeight()

    void getPositionInfo(wxPoint position, PointerPositionInfo& info) const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MODEL EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onVideoTracksAdded( model::EventAddVideoTracks& event );
    void onVideoTracksRemoved( model::EventRemoveVideoTracks& event );

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void draw(wxBitmap& bitmap) const; ///< @see View::draw()
};

}} // namespace

#endif // VIDEO_VIEW_H
