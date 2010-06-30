#ifndef PROJECT_COMMAND_ADD_ASSET_H
#define PROJECT_COMMAND_ADD_ASSET_H

#include "ProjectCommand.h"
#include "AProjectViewNode.h"

namespace command {

class ProjectCommandAddAsset : public ProjectView
{
public:
    /**
    * Add an existing asset which was removed/copied 
    */
    ProjectCommandAddAsset(model::ProjectViewPtr parent, model::ProjectViewPtrs nodes);

    ~ProjectCommandAddAsset();
    bool Do();
    bool Undo();
private:
    model::ProjectViewPtr mParent;
    model::ProjectViewPtrs mChildren;

};

} // namespace

#endif // PROJECT_COMMAND_ADD_ASSET_H
