// Copyright 2013,2014 Eric Raijmakers.
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

#include "Test.h"

namespace test {

gui::ProjectView& GetProjectView()
{
    return gui::ProjectView::get();
}

wxPoint ProjectViewPosition()
{
    return GetProjectView().GetScreenPosition();
}

model::FolderPtr ProjectViewAddAutoFolder( wxFileName path, model::FolderPtr parent )
{
    gui::Dialog::get().setDir( path.GetShortPath() ); // Add with short path to check that normalizing works
    WaitForIdle();
    util::thread::RunInMainAndWait([parent]
    {
        GetProjectView().select(boost::assign::list_of(parent));
        GetProjectView().onNewAutoFolder();
    });

    model::NodePtrs nodes = util::thread::RunInMainReturning<model::NodePtrs>(boost::bind(&model::Node::find, getRoot(), util::path::toPath(path))); // Converted to full path without trailing slash
    ASSERT_EQUALS(nodes.size(),1);
    model::NodePtr node = nodes.front();
    ASSERT(node->isA<model::AutoFolder>())(node);
    model::FolderPtr folder = boost::static_pointer_cast<model::Folder>(node);
    return folder;
}

model::FolderPtr ProjectViewAddFolder( wxString name, model::FolderPtr parent )
{
    gui::Dialog::get().setText( name );
    WaitForIdle();
    util::thread::RunInMainAndWait([parent]
    {
        GetProjectView().select(boost::assign::list_of(parent));
        GetProjectView().onNewFolder();
    });

    model::NodePtrs nodes = util::thread::RunInMainReturning<model::NodePtrs>(boost::bind(&model::Node::find, getRoot(), name));
    ASSERT_EQUALS(nodes.size(),1);
    model::NodePtr node = nodes.front();
    ASSERT(node->isA<model::Folder>())(node);
    model::FolderPtr folder = boost::static_pointer_cast<model::Folder>(node);
    return folder;
}

model::SequencePtr ProjectViewAddSequence( wxString name, model::FolderPtr parent )
{
    gui::Dialog::get().setText( name );
    WaitForIdle();
    util::thread::RunInMainAndWait([parent]
    {
        GetProjectView().select(boost::assign::list_of(parent));
        GetProjectView().onNewSequence();
    });

    model::NodePtrs nodes = util::thread::RunInMainReturning<model::NodePtrs>(boost::bind(&model::Node::find, getRoot(), name));
    ASSERT_EQUALS(nodes.size(),1);
    model::NodePtr node = nodes.front();
    ASSERT(node->isA<model::Sequence>())(node);
    model::SequencePtr sequence = boost::static_pointer_cast<model::Sequence>(node);
    return sequence;
}

model::SequencePtr ProjectViewCreateSequence( model::FolderPtr folder )
{
    WaitForIdle();
    util::thread::RunInMainAndWait([folder]
    {
        GetProjectView().select(boost::assign::list_of(folder));
        GetProjectView().onCreateSequence();
    });

    model::NodePtrs nodes;
    if (folder->getName().IsSameAs(folder->getSequenceName()))
    {
        nodes = util::thread::RunInMainReturning<model::NodePtrs>(boost::bind(&model::Node::find, getRoot(), folder->getName()));
        ASSERT_EQUALS(nodes.size(),2); // The sequence and the folder
    }
    else
    {
        nodes = util::thread::RunInMainReturning<model::NodePtrs>(boost::bind(&model::Node::find, getRoot(), folder->getName()));
        ASSERT_EQUALS(nodes.size(),1); // The folder
        nodes = getRoot()->find( folder->getSequenceName() );
        ASSERT_EQUALS(nodes.size(),1); // The sequence
    }
    model::SequencePtr result;
    for ( model::NodePtr node : nodes )
    {
        ASSERT(!result); // This code doesn't work when two sequences are part of the structure
        if (node->isA<model::Sequence>())
        {
            result = boost::static_pointer_cast<model::Sequence>(node);
        }
    }
    ASSERT(result);
    return result;
}

model::Files ProjectViewAddFiles( std::list<wxFileName> paths, model::FolderPtr parent )
{
    WaitForIdle();
    util::thread::RunInMainAndWait([parent]
    {
        GetProjectView().select(boost::assign::list_of(parent));
    });
    std::list<wxString> shortpaths;
    for ( wxFileName path : paths )
    {
        ASSERT( path.IsAbsolute() );
        shortpaths.push_back( path.GetShortPath() ); // Add with short path
    }
    gui::Dialog::get().setFiles( shortpaths );
    util::thread::RunInMainAndWait([]
    {
        GetProjectView().onNewFile();
    });

    model::Files result;
    for ( wxFileName path : paths )
    {
        model::NodePtrs nodes = util::thread::RunInMainReturning<model::NodePtrs>(boost::bind(&model::Node::find, parent, path.GetLongPath())); // Check that long path is generated by vidiot
        ASSERT_EQUALS( nodes.size(), 1 );
        model::NodePtr node = nodes.front();
        ASSERT(node->isA<model::File>())(node);
        result.push_back(boost::static_pointer_cast<model::File>(node));
    }
    return result;
}

void ProjectViewRemove( model::NodePtr node )
{
    util::thread::RunInMainAndWait([node]
    {
        GetProjectView().select(boost::assign::list_of(node));
        GetProjectView().onDelete();
    });
}

model::IPaths GetSupportedFiles( wxFileName directory )
{
    ASSERT(directory.IsDir() && directory.IsAbsolute())(directory);
    model::IPaths result;
    wxDir dir( directory.GetLongPath() );
    ASSERT(dir.IsOpened());
    wxString path;
    for (bool cont = dir.GetFirst(&path,wxEmptyString,wxDIR_FILES); cont; cont = dir.GetNext(&path))
    {
        wxFileName filename(directory.GetLongPath(), path);
        model::FilePtr file = boost::make_shared<model::File>(filename);
        if (file->canBeOpened())
        {
            result.push_back(file);
        }
    }
    return result;
}

int ProjectViewCount()
{
    WaitForIdle();
    util::thread::RunInMainAndWait([]
    {
        GetProjectView().selectAll();
    });
    model::NodePtrs selection = GetProjectView().getSelection();
    int result = selection.size();
    VAR_DEBUG(result);
    return result;
}

wxPoint ProjectViewFindNode( model::NodePtr node )
{
    return GetProjectView().find(node);
}

void ProjectViewMove(wxPoint position)
{
    MouseMoveWithinWidget(position, GetProjectView().GetScreenPosition());
}

wxPoint ProjectViewCenteredPosition( model::NodePtr node )
{
    return ProjectViewPosition() + ProjectViewFindNode( node );
}

void DragFromProjectViewToTimeline( model::NodePtr node, wxPoint to )
{
    ASSERT(FixtureGui::UseRealUiEvents);
    wxPoint position = ProjectViewCenteredPosition(node);
    ASSERT(!wxGetMouseState().LeftIsDown());
    MouseMoveOnScreen(position);
    MouseLeftDown();

    // Note 1: Need at least three consecutive drag events before the ProjectView decides that we're actually dragging. See ProjectView::onMotion.
    // Note 2: When DND is active (DoDragStart has been called) event handling is blocked. Therefore, WaitForIdle does not work below, until the drop is done (or the drag is aborted).
    int count = 0;
    SetWaitAfterEachInputAction(false);
    while (!gui::ProjectViewDropSource::get().isDragActive() && count++ < 100)
    {
        position.x += 1;
        MouseMoveOnScreen(position);
        pause(10);
    }
    ASSERT_LESS_THAN(count,100);

    // Drop onto target point
    count = 0;
    while (wxGetMouseState().GetPosition() != to && count++ < 3)
    {
        // Loop is required since sometimes the move fails the first time.
        // Particularly seen when working through remote desktop/using touchpad.
        MouseMoveOnScreen(to);
        pause(100); // Do not use waitforidle: does not work during drag and drop
    }
    ASSERT_LESS_THAN(count,3);

    wxUIActionSimulator().MouseUp();
    while (gui::ProjectViewDropSource::get().isDragActive())
    {
        pause(50); // Can't use WaitForIdle: event handling is blocked during DnD
    }
    SetWaitAfterEachInputAction(true);
    WaitForIdle(); // Can be used again when the DND is done.
}

void ProjectViewOpenTimelineForSequence(model::SequencePtr sequence)
{
    util::thread::RunInMainAndWait([sequence]
    {
        gui::ProjectView::get().select(boost::assign::list_of(boost::dynamic_pointer_cast<model::Node>(sequence)));
        GetProjectView().onOpen();
    });
}

WaitForChildCount::WaitForChildCount(model::NodePtr node, int count)
    :   mNode(node)
    ,   mCount(count)
    ,   mCountSeen(false)
{
    util::thread::RunInMainAndWait([this]
    {
        gui::Window::get().Bind(model::EVENT_ADD_NODE,     &WaitForChildCount::onNodeAdded,     this);
        gui::Window::get().Bind(model::EVENT_ADD_NODES,    &WaitForChildCount::onNodesAdded,    this);
        gui::Window::get().Bind(model::EVENT_REMOVE_NODE,  &WaitForChildCount::onNodeRemoved,   this);
        gui::Window::get().Bind(model::EVENT_REMOVE_NODES, &WaitForChildCount::onNodesRemoved,  this);
        mCountSeen = (mNode->count() == mCount);
        VAR_INFO(mCountSeen);
    });
    VAR_INFO(count);
    boost::mutex::scoped_lock lock(mMutex);
    while (!mCountSeen)
    {
        boost::system_time timeout = boost::get_system_time() + boost::posix_time::milliseconds(5000);
        if (!mCondition.timed_wait(lock,timeout))
        {
            FATAL(wxString::Format("Timeout: %d %d", mCount, mNode->count()));
        }
    }
}

WaitForChildCount::~WaitForChildCount()
{
    util::thread::RunInMainAndWait([this]
    {
        gui::Window::get().Unbind(model::EVENT_ADD_NODE,       &WaitForChildCount::onNodeAdded,      this);
        gui::Window::get().Unbind(model::EVENT_ADD_NODES,      &WaitForChildCount::onNodesAdded,     this);
        gui::Window::get().Unbind(model::EVENT_REMOVE_NODE,    &WaitForChildCount::onNodeRemoved,    this);
        gui::Window::get().Unbind(model::EVENT_REMOVE_NODES,   &WaitForChildCount::onNodesRemoved,   this);
    });
}

void WaitForChildCount::onNodeAdded(model::EventAddNode &event )
{
    event.Skip();
    check();
}

void  WaitForChildCount::onNodesAdded(model::EventAddNodes &event )
{
    event.Skip();
    check();
}

void WaitForChildCount::onNodeRemoved(model::EventRemoveNode &event )
{
    event.Skip();
    check();
}

void  WaitForChildCount::onNodesRemoved(model::EventRemoveNodes &event )
{
    event.Skip();
    check();
}

void WaitForChildCount::check()
{
    ASSERT(wxThread::IsMain());
    VAR_DEBUG(mNode->count());
    if (mNode->count() == mCount)
    {
        boost::mutex::scoped_lock lock(mMutex);
        mCountSeen = true;
        mCondition.notify_all();
    }
}

} // namespace
