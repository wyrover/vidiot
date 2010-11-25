#ifndef AUDIO_VIEW_H
#define AUDIO_VIEW_H

#include <wx/bitmap.h>
#include <wx/event.h>
#include "ModelPtr.h"
#include "GuiPtr.h"
#include "View.h"
#include "UtilEvent.h"

namespace gui { namespace timeline {

DECLARE_EVENT(AUDIO_UPDATE_EVENT, AudioUpdateEvent, AudioView*);

class TrackUpdateEvent;
struct PointerPositionInfo;

class AudioView
    :   public View
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    AudioView(Timeline* timeline);
    void init();
    virtual ~AudioView();

    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    /**
    * Overridden from View()
    **/
    int requiredWidth();

    /**
    * Overridden from View()
    **/
    int requiredHeight();

    void getPositionInfo(wxPoint position, PointerPositionInfo& info);

 private:

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onAudioTrackUpdated( TrackUpdateEvent& event );

    //////////////////////////////////////////////////////////////////////////
    // MODEL EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onAudioTracksAdded( model::EventAddAudioTracks& event );
    void onAudioTracksRemoved( model::EventRemoveAudioTracks& event );

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

     //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /**
    * Overridden from View()
    **/
    void draw(wxBitmap& bitmap);

};

}} // namespace

#endif // AUDIO_VIEW_H
