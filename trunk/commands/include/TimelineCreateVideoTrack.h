#ifndef PROJECT_COMMAND_CREATE_VIDEO_TRACK_H
#define PROJECT_COMMAND_CREATE_VIDEO_TRACK_H

#include "ProjectCommand.h"
#include "AProjectViewNode.h"
#include "Sequence.h"
#include "VideoTrack.h"

class ProjectCommandCreateVideoTrack : public ProjectCommand
{
public:
    ProjectCommandCreateVideoTrack(model::SequencePtr sequence);
    ~ProjectCommandCreateVideoTrack();
    bool Do();
    bool Undo();
private:
    model::SequencePtr mSequence;
    model::VideoTrackPtr mNewTrack;
};

#endif // PROJECT_COMMAND_CREATE_VIDEO_TRACK_H
