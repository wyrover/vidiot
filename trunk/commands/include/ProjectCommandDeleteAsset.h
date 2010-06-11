#ifndef PROJECT_COMMAND_DELETE_ASSET_H
#define PROJECT_COMMAND_DELETE_ASSET_H

#include "ProjectCommand.h"
#include "AProjectViewNode.h"

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

#endif // PROJECT_COMMAND_DELETE_ASSET_H
