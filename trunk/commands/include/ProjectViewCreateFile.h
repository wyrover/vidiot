#ifndef PROJECT_VIEW_CREATE_FILE_H
#define PROJECT_VIEW_CREATE_FILE_H

#include <boost/filesystem/path.hpp>
#include "ProjectViewCommand.h"
#include "Folder.h"
#include "File.h"

namespace command {

class ProjectViewCreateFile : public ProjectViewCommand
{
public:
    /** Add a new 'file' asset for each supplied file on disk */
    ProjectViewCreateFile(model::FolderPtr parent, std::vector<boost::filesystem::path> paths);

    ~ProjectViewCreateFile();
    bool Do();
    bool Undo();
private:
    std::vector<boost::filesystem::path> mPaths;
    model::FolderPtr mParent;
    std::vector<model::FilePtr> mChildren;
};

} // namespace

#endif // PROJECT_VIEW_CREATE_FILE_H
