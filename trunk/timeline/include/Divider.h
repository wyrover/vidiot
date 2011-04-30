#ifndef DIVIDER_H
#define DIVIDER_H

#include "View.h"

namespace model { 
    class EventAddVideoTracks; 
}

namespace gui { namespace timeline {
    class ViewUpdateEvent;
    struct PointerPositionInfo;

class Divider
    :   public View
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    Divider(View* parent);
    virtual ~Divider();

    //////////////////////////////////////////////////////////////////////////
    // MODEL EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onVideoTracksAdded( model::EventAddVideoTracks& event );

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    pixel requiredWidth() const;  ///< @see View::requiredWidth()
    pixel requiredHeight() const; ///< @see View::requiredHeight()

    void setPosition(int position);

    /// This method must be called whenever the number/height of the video
    /// and/or audio tracks changes. The new divider position will be calculated
    /// taking required defaults (like minimal areas around the audio/video 
    /// areas) into account.
    void resetDividerPosition();

    void getPositionInfo(wxPoint position, PointerPositionInfo& info ) const;

    int getVideoPosition() const;
    int getAudioPosition() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void draw(wxBitmap& bitmap) const; ///< @see View::draw()

};

}} // namespace

#endif // DIVIDER_H
