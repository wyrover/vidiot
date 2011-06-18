#ifndef PROJECT_VIEW_MOVE_ASSET_H
#define PROJECT_VIEW_MOVE_ASSET_H

#include "ProjectViewCommand.h"
#include "AProjectViewNode.h"

namespace command {

class ProjectViewMoveAsset : public ProjectViewCommand
{
public:
    ProjectViewMoveAsset(model::ProjectViewPtrs nodes, model::ProjectViewPtr parent);
    ~ProjectViewMoveAsset();
    bool Do();
    bool Undo();
private:
    ParentAndChildPairs mPairs;
    model::ProjectViewPtr mNewParent;
};

} // namespace

#endif // PROJECT_VIEW_MOVE_ASSET_H
