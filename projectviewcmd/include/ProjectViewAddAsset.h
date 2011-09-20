#ifndef PROJECT_VIEW_ADD_ASSET_H
#define PROJECT_VIEW_ADD_ASSET_H

#include "ProjectViewCommand.h"
#include "Node.h"

namespace command {

class ProjectViewAddAsset : public ProjectViewCommand
{
public:
    /**
    * Add an existing asset which was removed/copied 
    */
    ProjectViewAddAsset(model::NodePtr parent, model::NodePtrs nodes);

    ~ProjectViewAddAsset();
    bool Do() override;
    bool Undo() override;
private:
    model::NodePtr mParent;
    model::NodePtrs mChildren;

};

} // namespace

#endif // PROJECT_VIEW_ADD_ASSET_H
