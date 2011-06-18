#ifndef PROJECT_VIEW_ADD_ASSET_H
#define PROJECT_VIEW_ADD_ASSET_H

#include "ProjectViewCommand.h"
#include "AProjectViewNode.h"

namespace command {

class ProjectViewAddAsset : public ProjectViewCommand
{
public:
    /**
    * Add an existing asset which was removed/copied 
    */
    ProjectViewAddAsset(model::ProjectViewPtr parent, model::ProjectViewPtrs nodes);

    ~ProjectViewAddAsset();
    bool Do();
    bool Undo();
private:
    model::ProjectViewPtr mParent;
    model::ProjectViewPtrs mChildren;

};

} // namespace

#endif // PROJECT_VIEW_ADD_ASSET_H
