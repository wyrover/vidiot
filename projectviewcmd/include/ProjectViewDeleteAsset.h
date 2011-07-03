#ifndef PROJECT_VIEW_DELETE_ASSET_H
#define PROJECT_VIEW_DELETE_ASSET_H

#include "ProjectViewCommand.h"
#include "AProjectViewNode.h"

namespace command {

class ProjectViewDeleteAsset : public ProjectViewCommand
{
public:
    ProjectViewDeleteAsset(model::NodePtrs nodes);
    ~ProjectViewDeleteAsset();
    bool Do();
    bool Undo();
private:
    ParentAndChildPairs mPairs;
};

} // namespace

#endif // PROJECT_VIEW_DELETE_ASSET_H
