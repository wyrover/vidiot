#ifndef PROJECT_COMMAND_DELETE_ASSET_H
#define PROJECT_COMMAND_DELETE_ASSET_H

#include "ProjectCommand.h"
#include "AProjectViewNode.h"

namespace command {

class ProjectCommandDeleteAsset : public ProjectCommand
{
public:
    ProjectCommandDeleteAsset(model::ProjectViewPtrs nodes);
    ~ProjectCommandDeleteAsset();
    bool Do();
    bool Undo();
private:
    ParentAndChildPairs mPairs;
};

} // namespace

#endif // PROJECT_COMMAND_DELETE_ASSET_H
