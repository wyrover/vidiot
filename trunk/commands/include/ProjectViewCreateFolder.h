#ifndef PROJECT_COMMAND_CREATE_FOLDER_H
#define PROJECT_COMMAND_CREATE_FOLDER_H

#include "ProjectCommand.h"
#include "Folder.h"

namespace command {

class ProjectCommandCreateFolder : public ProjectCommand
{
public:
    /** Create a new folder in another folder. */
    ProjectCommandCreateFolder(model::FolderPtr parent, wxString name);
    ~ProjectCommandCreateFolder();
    bool Do();
    bool Undo();
private:
    model::FolderPtr mParent;
    model::FolderPtr mNewFolder;
    wxString mName;
};

} // namespace

#endif // PROJECT_COMMAND_CREATE_FOLDER_H
