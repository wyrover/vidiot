#ifndef TIMELINE_CREATE_AUDIO_TRACK_H
#define TIMELINE_CREATE_AUDIO_TRACK_H

#include "TimelineCommand.h"
#include "Sequence.h"
#include "AudioTrack.h"

namespace command {

class TimelineCreateAudioTrack : public TimelineCommand
{
public:
    TimelineCreateAudioTrack(model::SequencePtr sequence);
    ~TimelineCreateAudioTrack();
    bool Do();
    bool Undo();
private:
    model::SequencePtr mSequence;
    model::AudioTrackPtr mNewTrack;
};

} // namespace

#endif // TIMELINE_CREATE_AUDIO_TRACK_H
