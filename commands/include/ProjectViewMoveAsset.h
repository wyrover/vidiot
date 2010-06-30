#ifndef PROJECT_COMMAND_MOVE_ASSET_H
#define PROJECT_COMMAND_MOVE_ASSET_H

#include "ProjectCommand.h"
#include "AProjectViewNode.h"

namespace command {

class ProjectCommandMoveAsset : public ProjectCommand
{
public:
    ProjectCommandMoveAsset(model::ProjectViewPtrs nodes, model::ProjectViewPtr parent);
    ~ProjectCommandMoveAsset();
    bool Do();
    bool Undo();
private:
    ParentAndChildPairs mPairs;
    model::ProjectViewPtr mNewParent;
};

} // namespace

#endif // PROJECT_COMMAND_MOVE_ASSET_H
