#ifndef PROJECT_COMMAND_CREATE_AUTOFOLDER_H
#define PROJECT_COMMAND_CREATE_AUTOFOLDER_H

#include <boost/filesystem/path.hpp>
#include "ProjectCommand.h"
#include "AutoFolder.h"

namespace command {

class ProjectCommandCreateAutoFolder : public ProjectCommand
{
public:
    ProjectCommandCreateAutoFolder(model::FolderPtr parent, boost::filesystem::path path);
    ~ProjectCommandCreateAutoFolder();
    bool Do();
    bool Undo();
private:
    model::FolderPtr mParent;
    model::AutoFolderPtr mNewAutoFolder;
    boost::filesystem::path  mPath;
};

} // namespace

#endif // PROJECT_COMMAND_CREATE_AUTOFOLDER_H
