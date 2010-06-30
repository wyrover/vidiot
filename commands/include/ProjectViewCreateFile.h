#ifndef PROJECT_COMMAND_CREATE_FILE_H
#define PROJECT_COMMAND_CREATE_FILE_H

#include <boost/filesystem/path.hpp>
#include "ProjectCommand.h"
#include "Folder.h"
#include "File.h"

namespace command {

class ProjectCommandCreateFile : public ProjectCommand
{
public:
    /** Add a new 'file' asset for each supplied file on disk */
    ProjectCommandCreateFile(model::FolderPtr parent, std::vector<boost::filesystem::path> paths);

    ~ProjectCommandCreateFile();
    bool Do();
    bool Undo();
private:
    std::vector<boost::filesystem::path> mPaths;
    model::FolderPtr mParent;
    std::vector<model::FilePtr> mChildren;
};

} // namespace

#endif // PROJECT_COMMAND_CREATE_FILE_H
