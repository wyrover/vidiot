#ifndef CREATE_AUDIO_TRACK_H
#define CREATE_AUDIO_TRACK_H

#include "ATimelineCommand.h"
#include "ModelPtr.h"

namespace gui { namespace timeline { namespace command {

class CreateAudioTrack 
    :   public ATimelineCommand
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    CreateAudioTrack (gui::timeline::Timeline& timeline);
    ~CreateAudioTrack ();
    
    //////////////////////////////////////////////////////////////////////////
    // WXWIDGETS DO/UNDO INTERFACE
    //////////////////////////////////////////////////////////////////////////

    bool Do();
    bool Undo();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::AudioTrackPtr mNewTrack;
};

}}} // namespace

#endif // CREATE_AUDIO_TRACK_H