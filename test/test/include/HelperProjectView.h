// Copyright 2013 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#ifndef HELPER_PROJECT_VIEW_H
#define HELPER_PROJECT_VIEW_H

#include <list>
#include <wx/filename.h>
#include <boost/shared_ptr.hpp>
#include "HelperWindow.h"
#include "ProjectView.h"

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

class EventAddNode;
class EventAddNodes;
class EventRemoveNode;
class EventRemoveNodes;
}

namespace test {

/// \return the project view
gui::ProjectView& getProjectView();

/// \return the position of the project view on screen
wxPoint ProjectViewPosition();

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

/// \return list of supported files in the given directory. Folders are returned also.
/// \param path absolute path which is searched for files.
/// Furthermore, note that files are opened with avcodec. If that returns an error, the file
/// is not added to the returned list - see File::canBeOpened().
model::IPaths getSupportedFiles( wxFileName path );

/// Count the number of nodes currently visible in the project view.
/// This is implemented by selecting all nodes, and counting the selection size.
/// Thus, as a side effect changes the selection of the project view.
int countProjectView();

/// \return the coordinates of the given node
/// \param node node to be found
wxPoint findNode( model::NodePtr node );

/// \return center position of given node, in screen coordinates
wxPoint CenterInProjectView(model::NodePtr node);

/// Perform a drag and drop operation from project view to the timeline
/// \param from position within project view
/// \param to position within timeline
void DragFromProjectViewToTimeline(model::NodePtr node, wxPoint to);

/// Delays execution of the module test until a given node has signaled (via an event)
/// that it has the given number of children.
struct WaitForChildCount
{
    WaitForChildCount(model::NodePtr, int count);
    virtual ~WaitForChildCount();

    void onNodeAdded( model::EventAddNode &event );
    void onNodesAdded( model::EventAddNodes &event );
    void onNodeRemoved( model::EventRemoveNode &event );
    void onNodesRemoved( model::EventRemoveNodes &event );

private:

    void check();

    model::NodePtr mNode;
    int mCount;

    bool mCountSeen;

    boost::mutex mMutex;
    boost::condition_variable mCondition;
};

} // namespace

#endif // HELPER_PROJECT_VIEW_H