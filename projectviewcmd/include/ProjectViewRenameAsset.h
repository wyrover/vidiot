#ifndef PROJECT_VIEW_RENAME_ASSET_H
#define PROJECT_VIEW_RENAME_ASSET_H

#include "ProjectViewCommand.h"
#include "AProjectViewNode.h"

namespace command {

class ProjectViewRenameAsset : public ProjectViewCommand
{
public:
    ProjectViewRenameAsset(model::NodePtr node, wxString assetName);
    ~ProjectViewRenameAsset();
    bool Do();
    bool Undo();
private:
    model::NodePtr mNode;
    wxString mNewName;
    wxString mOldName;
};

} // namespace

#endif // PROJECT_VIEW_RENAME_ASSET_H
