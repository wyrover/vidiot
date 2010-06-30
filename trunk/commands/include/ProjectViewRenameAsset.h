#ifndef PROJECT_COMMAND_RENAME_ASSET_H
#define PROJECT_COMMAND_RENAME_ASSET_H

#include "ProjectCommand.h"
#include "AProjectViewNode.h"

namespace command {

class ProjectCommandRenameAsset : public ProjectCommand
{
public:
    ProjectCommandRenameAsset(model::ProjectViewPtr node, wxString assetName);
    ~ProjectCommandRenameAsset();
    bool Do();
    bool Undo();
private:
    model::ProjectViewPtr mNode;
    wxString mNewName;
    wxString mOldName;
};

} // namespace

#endif // PROJECT_COMMAND_RENAME_ASSET_H
