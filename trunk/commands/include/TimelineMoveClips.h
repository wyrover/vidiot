#ifndef TIMELINE_MOVE_CLIPS_H
#define TIMELINE_MOVE_CLIPS_H

#include <set>
#include "TimelineCommand.h"
#include "ModelPtr.h"
#include "Track.h"

namespace command {

class TimelineMoveClips 
    :   public TimelineCommand
{
public:

    TimelineMoveClips(gui::timeline::Timeline& timeline, model::MoveParameters params);
    ~TimelineMoveClips();
    bool Do();
    bool Undo();
private:
    model::MoveParameters mParams;
    model::MoveParameters mParamsUndo;

    void doMove(model::MoveParameterPtr move);
};

} // namespace

#endif // TIMELINE_MOVE_CLIPS_H
