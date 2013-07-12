#include "HelperProjectView.h"

#include "AutoFolder.h"
#include "Dialog.h"
#include "File.h"
#include "HelperApplication.h"
#include "HelperTimeline.h"
#include "HelperTimelinesView.h"
#include "ids.h"
#include "NodeEvent.h"
#include "ProjectView.h"
#include "Sequence.h"
#include "Timeline.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "UtilPath.h"
#include "UtilThread.h"
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <wx/uiaction.h>

namespace test {

gui::ProjectView& getProjectView()
{
    return gui::ProjectView::get();
}

wxPoint ProjectViewPosition()
{
    return getProjectView().GetScreenPosition();
}

model::FolderPtr addAutoFolder( wxFileName path, model::FolderPtr parent )
{
    waitForIdle();
    util::thread::RunInMainAndWait([parent]
    {
        getProjectView().select(boost::assign::list_of(parent));
    });
    waitForIdle();
    gui::Dialog::get().setDir( path.GetShortPath() ); // Add with short path to check that normalizing works
    triggerMenu(gui::ProjectView::get(),meID_NEW_AUTOFOLDER);

    model::NodePtrs nodes = util::thread::RunInMainReturning<model::NodePtrs>(boost::bind(&model::Node::find, getRoot(), util::path::toPath(path))); // Converted to full path without trailing slash
    ASSERT_EQUALS(nodes.size(),1);
    model::NodePtr node = nodes.front();
    ASSERT(node->isA<model::AutoFolder>())(node);
    model::FolderPtr folder = boost::static_pointer_cast<model::Folder>(node);
    return folder;
}

model::FolderPtr addFolder( wxString name, model::FolderPtr parent )
{
    waitForIdle();
    util::thread::RunInMainAndWait([parent]
    {
        getProjectView().select(boost::assign::list_of(parent));
    });
    gui::Dialog::get().setText( name );
    triggerMenu(gui::ProjectView::get(),meID_NEW_FOLDER);
    waitForIdle();

    model::NodePtrs nodes = util::thread::RunInMainReturning<model::NodePtrs>(boost::bind(&model::Node::find, getRoot(), name));
    ASSERT_EQUALS(nodes.size(),1);
    model::NodePtr node = nodes.front();
    ASSERT(node->isA<model::Folder>())(node);
    model::FolderPtr folder = boost::static_pointer_cast<model::Folder>(node);
    return folder;
}

model::SequencePtr addSequence( wxString name, model::FolderPtr parent )
{
    waitForIdle();
    util::thread::RunInMainAndWait([parent]
    {
        getProjectView().select(boost::assign::list_of(parent));
    });
    gui::Dialog::get().setText( name );
    triggerMenu(gui::ProjectView::get(),meID_NEW_SEQUENCE);
    waitForIdle();

    model::NodePtrs nodes = util::thread::RunInMainReturning<model::NodePtrs>(boost::bind(&model::Node::find, getRoot(), name));
    ASSERT_EQUALS(nodes.size(),1);
    model::NodePtr node = nodes.front();
    ASSERT(node->isA<model::Sequence>())(node);
    model::SequencePtr sequence = boost::static_pointer_cast<model::Sequence>(node);
    return sequence;
}

model::SequencePtr createSequence( model::FolderPtr folder )
{
    waitForIdle();
    util::thread::RunInMainAndWait([folder]
    {
        getProjectView().select(boost::assign::list_of(folder));
    });
    waitForIdle();
    triggerMenu(getProjectView(),meID_CREATE_SEQUENCE);
    waitForIdle();

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
    BOOST_FOREACH( model::NodePtr node, nodes )
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

model::Files addFiles( std::list<wxFileName> paths, model::FolderPtr parent )
{
    waitForIdle();
    util::thread::RunInMainAndWait([parent]
    {
        getProjectView().select(boost::assign::list_of(parent));
    });
    std::list<wxString> shortpaths;
    BOOST_FOREACH( wxFileName path, paths )
    {
        ASSERT( path.IsAbsolute() );
        shortpaths.push_back( path.GetShortPath() ); // Add with short path
    }
    gui::Dialog::get().setFiles( shortpaths );
    triggerMenu(gui::ProjectView::get(),meID_NEW_FILE);
    waitForIdle();

    model::Files result;
    BOOST_FOREACH( wxFileName path, paths )
    {
        model::NodePtrs nodes = util::thread::RunInMainReturning<model::NodePtrs>(boost::bind(&model::Node::find, parent, path.GetLongPath())); // Check that long path is generated by vidiot
        ASSERT_EQUALS( nodes.size(), 1 );
        model::NodePtr node = nodes.front();
        ASSERT(node->isA<model::File>())(node);
        result.push_back(boost::static_pointer_cast<model::File>(node));
    }
    return result;
}

void remove( model::NodePtr node )
{
    waitForIdle();
    util::thread::RunInMainAndWait([node]
    {
        getProjectView().select(boost::assign::list_of(node));
    });
    waitForIdle();
    triggerMenu(gui::ProjectView::get(),wxID_DELETE);
    waitForIdle();
}

model::IPaths getSupportedFiles( wxFileName directory )
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

int countProjectView()
{
    waitForIdle();
    util::thread::RunInMainAndWait([]
    {
        getProjectView().selectAll();
    });
    model::NodePtrs selection = getProjectView().getSelection();
    int result = selection.size();
    VAR_DEBUG(result);
    return result;
}

wxPoint findNode( model::NodePtr node )
{
    return getProjectView().find(node);
}

void MoveProjectView(wxPoint position)
{
    MoveWithinWidget(position, getProjectView().GetScreenPosition());
}

wxPoint CenterInProjectView( model::NodePtr node )
{
    return ProjectViewPosition() + findNode( node );
}

void DragFromProjectViewToTimeline( model::NodePtr node, wxPoint to )
{
    wxPoint position = CenterInProjectView(node);

    ASSERT(!wxGetMouseState().LeftDown());
    MoveOnScreen(position);
    LeftDown();

    // Note 1: Need at least three consecutive drag events before the ProjectView decides that we're actually dragging. See ProjectView::onMotion.
    // Note 2: When DND is active (DoDragStart has been called) event handling is blocked. Therefore, waitForIdle does not work below, until the drop is done (or the drag is aborted).
    int count = 0;
    while (!gui::ProjectViewDropSource::get().isDragActive() && count++ < 100)
    {
        position.x += 1;
        wxUIActionSimulator().MouseMove(position);
        pause(10);
    }
    ASSERT_LESS_THAN(count,100);

    // Drop onto target point
    count = 0;
    while (wxGetMouseState().GetPosition() != to && count++ < 3)
    {
        // Loop is required since sometimes the move fails the first time.
        // Particularly seen when working through remote desktop/using touchpad.
        wxUIActionSimulator().MouseMove(to);
        pause(100); // Do not use waitforidle: does not work during drag and drop
    }
    ASSERT_LESS_THAN(count,3);

    wxUIActionSimulator().MouseUp();
    while (gui::ProjectViewDropSource::get().isDragActive())
    {
        pause(50); // Can't use waitForIdle: event handling is blocked during DnD
    }
    waitForIdle(); // Can be used again when the DND is done.
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
            FATAL("Timeout");
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

void WaitForChildCount::onNodeAdded( model::EventAddNode &event )
{
    event.Skip();
    check();
}

void  WaitForChildCount::onNodesAdded( model::EventAddNodes &event )
{
    event.Skip();
    check();
}

void WaitForChildCount::onNodeRemoved( model::EventRemoveNode &event )
{
    event.Skip();
    check();
}

void  WaitForChildCount::onNodesRemoved( model::EventRemoveNodes &event )
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