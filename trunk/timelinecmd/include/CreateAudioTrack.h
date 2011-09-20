#ifndef CREATE_AUDIO_TRACK_H
#define CREATE_AUDIO_TRACK_H

#include <boost/shared_ptr.hpp>
#include "ATimelineCommand.h"

namespace model {
class AudioTrack;
typedef boost::shared_ptr<AudioTrack> AudioTrackPtr;
}

namespace gui { namespace timeline { namespace command {

class CreateAudioTrack 
    :   public ATimelineCommand
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    explicit CreateAudioTrack (model::SequencePtr sequence);
    ~CreateAudioTrack ();

    //////////////////////////////////////////////////////////////////////////
    // WXWIDGETS DO/UNDO INTERFACE
    //////////////////////////////////////////////////////////////////////////

    bool Do() override;
    bool Undo() override;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::AudioTrackPtr mNewTrack;
};

}}} // namespace

#endif // CREATE_AUDIO_TRACK_H
