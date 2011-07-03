#ifndef PROJECT_VIEW_MOVE_ASSET_H
#define PROJECT_VIEW_MOVE_ASSET_H

#include "ProjectViewCommand.h"
#include "AProjectViewNode.h"

namespace command {

class ProjectViewMoveAsset : public ProjectViewCommand
{
public:
    ProjectViewMoveAsset(model::NodePtrs nodes, model::NodePtr parent);
    ~ProjectViewMoveAsset();
    bool Do();
    bool Undo();
private:
    ParentAndChildPairs mPairs;
    model::NodePtr mNewParent;
};

} // namespace

#endif // PROJECT_VIEW_MOVE_ASSET_H
