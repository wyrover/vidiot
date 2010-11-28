#ifndef AUDIO_VIEW_H
#define AUDIO_VIEW_H

#include "View.h"

namespace model {
    class EventAddAudioTracks;
    class EventRemoveAudioTracks;
}

namespace gui { namespace timeline {
struct PointerPositionInfo;

class AudioView
    :   public View
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    AudioView(View* parent);
    virtual ~AudioView();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    int requiredWidth();  ///< @see View::requiredWidth()
    int requiredHeight(); ///< @see View::requiredHeight()

    void getPositionInfo(wxPoint position, PointerPositionInfo& info);

private:

    //////////////////////////////////////////////////////////////////////////
    // MODEL EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onAudioTracksAdded( model::EventAddAudioTracks& event );
    void onAudioTracksRemoved( model::EventRemoveAudioTracks& event );

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void draw(wxBitmap& bitmap); ///< @see View::draw()

};

}} // namespace

#endif // AUDIO_VIEW_H
