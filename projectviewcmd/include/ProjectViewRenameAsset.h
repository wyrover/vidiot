#ifndef PROJECT_VIEW_RENAME_ASSET_H
#define PROJECT_VIEW_RENAME_ASSET_H

#include "ProjectViewCommand.h"
#include "Node.h"

namespace command {

class ProjectViewRenameAsset : public ProjectViewCommand
{
public:
    ProjectViewRenameAsset(model::NodePtr node, wxString assetName);
    ~ProjectViewRenameAsset();
    bool Do() override;
    bool Undo() override;
private:
    model::NodePtr mNode;
    wxString mNewName;
    wxString mOldName;
};

} // namespace

#endif // PROJECT_VIEW_RENAME_ASSET_H
