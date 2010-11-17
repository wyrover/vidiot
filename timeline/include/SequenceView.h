#ifndef SEQUENCE_VIEW_H
#define SEQUENCE_VIEW_H

#include <wx/bitmap.h>
#include <wx/event.h>
#include "ModelPtr.h"
#include "GuiPtr.h"
#include "Part.h"
#include "UtilEvent.h"

namespace gui { namespace timeline {

DECLARE_EVENT(VIDEO_UPDATE_EVENT, VideoUpdateEvent, SequenceView*);
DECLARE_EVENT(AUDIO_UPDATE_EVENT, AudioUpdateEvent, SequenceView*);

class TrackUpdateEvent;
struct PointerPositionInfo;

class SequenceView
    :   public wxEvtHandler
    ,   public Part
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION METHODS
    //////////////////////////////////////////////////////////////////////////

    SequenceView();
    void init();
    virtual ~SequenceView();

    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    int requiredWidth();
    int requiredVideoHeight();
    int requiredAudioHeight();

    void getPositionInfo(wxPoint position, PointerPositionInfo& info);

    const wxBitmap& getVideo() const;
    const wxBitmap& getAudio() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onVideoTrackUpdated( TrackUpdateEvent& event );
    void onAudioTrackUpdated( TrackUpdateEvent& event );

    //////////////////////////////////////////////////////////////////////////
    // MODEL EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onVideoTracksAdded( model::EventAddVideoTracks& event );
    void onVideoTracksRemoved( model::EventRemoveVideoTracks& event );
    void onAudioTracksAdded( model::EventAddAudioTracks& event );
    void onAudioTracksRemoved( model::EventRemoveAudioTracks& event );

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxBitmap mVideo;
    wxBitmap mAudio;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void updateVideoSize();
    void updateAudioSize();
    void updateVideoBitmap();
    void updateAudioBitmap();
};

}} // namespace

#endif // SEQUENCE_VIEW_H
