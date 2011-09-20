#ifndef PROJECT_VIEW_CREATE_FOLDER_H
#define PROJECT_VIEW_CREATE_FOLDER_H

#include "ProjectViewCommand.h"
#include "Folder.h"

namespace command {

class ProjectViewCreateFolder : public ProjectViewCommand
{
public:
    /** Create a new folder in another folder. */
    ProjectViewCreateFolder(model::FolderPtr parent, wxString name);
    ~ProjectViewCreateFolder();
    bool Do() override;
    bool Undo() override;
private:
    model::FolderPtr mParent;
    model::FolderPtr mNewFolder;
    wxString mName;
};

} // namespace

#endif // PROJECT_VIEW_CREATE_FOLDER_H
