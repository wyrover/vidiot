#ifndef PROJECT_VIEW_CREATE_AUTOFOLDER_H
#define PROJECT_VIEW_CREATE_AUTOFOLDER_H

#include <boost/filesystem/path.hpp>
#include "ProjectViewCommand.h"
#include "AutoFolder.h"

namespace command {

class ProjectViewCreateAutoFolder 
    :   public ProjectViewCommand
{
public:
    ProjectViewCreateAutoFolder(model::FolderPtr parent, boost::filesystem::path path);
    ~ProjectViewCreateAutoFolder();
    bool Do();
    bool Undo();
private:
    model::FolderPtr mParent;
    model::AutoFolderPtr mNewAutoFolder;
    boost::filesystem::path  mPath;
};

} // namespace

#endif // PROJECT_VIEW_CREATE_AUTOFOLDER_H
