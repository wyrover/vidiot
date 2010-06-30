#ifndef PROJECT_COMMAND_MOVE_CLIPS_H
#define PROJECT_COMMAND_MOVE_CLIPS_H

#include "ProjectCommand.h"
#include "ModelPtr.h"

namespace command {

class ProjectCommandMoveClips : public ProjectCommand
{
public:
    ProjectCommandMoveClips(model::ProjectViewPtrs nodes, model::ProjectViewPtr parent);
    ~ProjectCommandMoveClips();
    bool Do();
    bool Undo();
private:
    ParentAndChildPairs mPairs;
    model::ProjectViewPtr mNewParent;
};

} // namespace

#endif // PROJECT_COMMAND_MOVE_CLIPS_H
