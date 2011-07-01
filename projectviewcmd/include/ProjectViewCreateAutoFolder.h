#ifndef PROJECT_VIEW_CREATE_AUTOFOLDER_H
#define PROJECT_VIEW_CREATE_AUTOFOLDER_H

#include <wx/filename.h>
#include "ProjectViewCommand.h"

namespace model {
class Folder;
typedef boost::shared_ptr<Folder> FolderPtr;
class AutoFolder;
typedef boost::shared_ptr<AutoFolder> AutoFolderPtr;
}

namespace command {

class ProjectViewCreateAutoFolder 
    :   public ProjectViewCommand
{
public:
    ProjectViewCreateAutoFolder(model::FolderPtr parent, wxFileName path);
    ~ProjectViewCreateAutoFolder();
    bool Do();
    bool Undo();
private:
    model::FolderPtr mParent;
    model::AutoFolderPtr mNewAutoFolder;
    wxFileName mPath;
};

} // namespace

#endif // PROJECT_VIEW_CREATE_AUTOFOLDER_H
