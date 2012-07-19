#ifndef PROJECT_VIEW_CREATE_FILE_H
#define PROJECT_VIEW_CREATE_FILE_H

#include "ProjectViewCommand.h"

namespace model {
class Folder;
typedef boost::shared_ptr<Folder> FolderPtr;
class File;
typedef boost::shared_ptr<File> FilePtr;
typedef std::list<FilePtr> Files;
}
namespace command {

class ProjectViewCreateFile
    :   public ProjectViewCommand
{
public:
    /// Add a new 'file' asset for each supplied file on disk
    /// \param parent node to use for new assets
    /// \param paths list of full paths to files
    ProjectViewCreateFile(model::FolderPtr parent, std::vector<wxFileName> paths);
    ~ProjectViewCreateFile();
    bool Do() override;
    bool Undo() override;
private:
    std::vector<wxFileName> mPaths;
    model::FolderPtr mParent;
    std::vector<model::FilePtr> mChildren;
};

} // namespace

#endif // PROJECT_VIEW_CREATE_FILE_H