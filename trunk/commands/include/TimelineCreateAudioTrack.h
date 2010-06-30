#ifndef PROJECT_COMMAND_CREATE_AUDIO_TRACK_H
#define PROJECT_COMMAND_CREATE_AUDIO_TRACK_H

#include "ProjectCommand.h"
#include "AProjectViewNode.h"
#include "Sequence.h"
#include "AudioTrack.h"

class ProjectCommandCreateAudioTrack : public ProjectCommand
{
public:
    ProjectCommandCreateAudioTrack(model::SequencePtr sequence);
    ~ProjectCommandCreateAudioTrack();
    bool Do();
    bool Undo();
private:
    model::SequencePtr mSequence;
    model::AudioTrackPtr mNewTrack;
};

#endif // PROJECT_COMMAND_CREATE_AUDIO_TRACK_H
