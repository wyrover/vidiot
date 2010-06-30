#ifndef TIMELINE_MOVE_CLIPS_H
#define TIMELINE_MOVE_CLIPS_H

#include "TimelineCommand.h"
#include "ModelPtr.h"

namespace command {

class TimelineMoveClips : public TimelineCommand
{
public:
    TimelineMoveClips();
    ~TimelineMoveClips();
    bool Do();
    bool Undo();
};

} // namespace

#endif // TIMELINE_MOVE_CLIPS_H
