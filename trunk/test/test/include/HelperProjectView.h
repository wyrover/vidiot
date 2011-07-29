#ifndef HELPER_PROJECT_VIEW_H
#define HELPER_PROJECT_VIEW_H

#include <list>
#include <wx/filename.h>
#include <boost/shared_ptr.hpp>
#include "HelperWindow.h"

namespace model {
class INode;
typedef boost::shared_ptr<INode> NodePtr;
class Folder;
typedef boost::shared_ptr<Folder> FolderPtr;
class Sequence;
typedef boost::shared_ptr<Sequence> SequencePtr;
class File;
typedef boost::shared_ptr<File> FilePtr;
typedef std::list<FilePtr> Files;
}

namespace test {

/// Create a new autofolder to the given path in a given parent folder or in the root (default)
/// \return new autofolder
model::FolderPtr addAutoFolder( wxFileName path, model::FolderPtr parent = getRoot() );

/// Create a new named folder in a given parent folder or in the root (default)
/// \return new folder
model::FolderPtr addFolder( wxString name, model::FolderPtr parent = getRoot() );

/// Create a new sequence in a given parent folder or in the root (default)
/// \return new sequence
model::SequencePtr addSequence( wxString name, model::FolderPtr parent = getRoot() );

/// Create a new sequence from a given folder
/// \return new sequence
model::SequencePtr createSequence( model::FolderPtr folder );

/// Create new files in a given parent folder or in the root (default)
/// \return new files created in the model
model::Files addFiles( std::list<wxFileName> name, model::FolderPtr parent = getRoot() );

/// Remove given node from the project view via selecting it and then triggering the delete menu option
/// \node node to be removed
void remove( model::NodePtr node );

/// Count the number of nodes currently visible in the project view.
/// This is implemented by selecting all nodes, and counting the selection size.
/// Thus, as a side effect changes the selection of the project view.
int countProjectView();

} // namespace

#endif // HELPER_PROJECT_VIEW_H