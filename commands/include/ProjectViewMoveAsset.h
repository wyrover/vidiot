#ifndef PROJECT_COMMAND_MOVE_ASSET_H
#define PROJECT_COMMAND_MOVE_ASSET_H

#include "ProjectCommand.h"
#include "AProjectViewNode.h"

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

#endif // PROJECT_COMMAND_MOVE_ASSET_H
