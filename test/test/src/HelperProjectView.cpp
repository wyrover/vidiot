#include "HelperProjectView.h"

#include <wx/uiaction.h>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include "AutoFolder.h"
#include "File.h"
#include "HelperApplication.h"
#include "HelperTimeline.h"
#include "HelperTimelinesView.h"
#include "ids.h"
#include "ProjectView.h"
#include "Timeline.h"
#include "Sequence.h"
#include "Dialog.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"

namespace test {

gui::ProjectView& getProjectView()
{
    return gui::ProjectView::get();
}

model::FolderPtr addAutoFolder( wxFileName path, model::FolderPtr parent )
{
    waitForIdle();
    getProjectView().select(boost::assign::list_of(parent));
    waitForIdle();
    gui::Dialog::get().setDir( path.GetShortPath() ); // Add with short path
    triggerMenu(gui::ProjectView::get(),meID_NEW_AUTOFOLDER);
    waitForIdle();

    model::NodePtrs nodes = getRoot()->find( path.GetLongPath() ); // Converted to long path in vidiot
    ASSERT_EQUALS(nodes.size(),1);
    model::NodePtr node = nodes.front();
    ASSERT(node->isA<model::AutoFolder>())(node);
    model::FolderPtr folder = boost::static_pointer_cast<model::Folder>(node);
    return folder;
}

model::FolderPtr addFolder( wxString name, model::FolderPtr parent )
{
    waitForIdle();
    getProjectView().select(boost::assign::list_of(parent));
    gui::Dialog::get().setText( name );
    triggerMenu(gui::ProjectView::get(),meID_NEW_FOLDER);
    waitForIdle();

    model::NodePtrs nodes = getRoot()->find( name );
    ASSERT_EQUALS(nodes.size(),1);
    model::NodePtr node = nodes.front();
    ASSERT(node->isA<model::Folder>())(node);
    model::FolderPtr folder = boost::static_pointer_cast<model::Folder>(node);
    return folder;
}

model::SequencePtr addSequence( wxString name, model::FolderPtr parent )
{
    waitForIdle();
    getProjectView().select(boost::assign::list_of(parent));
    gui::Dialog::get().setText( name );
    triggerMenu(gui::ProjectView::get(),meID_NEW_SEQUENCE);
    waitForIdle();

    model::NodePtrs nodes = getRoot()->find( name );
    ASSERT_EQUALS(nodes.size(),1);
    model::NodePtr node = nodes.front();
    ASSERT(node->isA<model::Sequence>())(node);
    model::SequencePtr sequence = boost::static_pointer_cast<model::Sequence>(node);
    return sequence;
}

model::SequencePtr createSequence( model::FolderPtr folder )
{
    waitForIdle();
    getProjectView().select(boost::assign::list_of(folder));
    waitForIdle();
    triggerMenu(getProjectView(),meID_CREATE_SEQUENCE);
    waitForIdle();

    model::NodePtrs nodes;
    if (folder->getName().IsSameAs(folder->getSequenceName()))
    {
        nodes = getRoot()->find( folder->getName() );
        ASSERT_EQUALS(nodes.size(),2); // The sequence and the folder
    }
    else
    {
        nodes = getRoot()->find( folder->getName() );
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
    getProjectView().select(boost::assign::list_of(parent));
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
        model::NodePtrs nodes = parent->find( path.GetLongPath() ); // Check that long path is generated by vidiot
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
    getProjectView().select(boost::assign::list_of(node));
    waitForIdle();
    triggerMenu(gui::ProjectView::get(),wxID_DELETE);
    waitForIdle();
}

int countProjectView()
{
    waitForIdle();
    getProjectView().selectAll();
    model::NodePtrs selection =getProjectView().getSelection();
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

void DragFromProjectViewToTimeline(wxPoint from, wxPoint to)
{
    FATAL("Does not work yet.");
    VAR_DEBUG(from)(to);
    MoveProjectView(from)   ;
    wxUIActionSimulator().MouseDown();
    waitForIdle();
    wxPoint fromAbs = from + getProjectView().GetScreenPosition();
    wxPoint toAbs = to + getTimeline().GetScreenPosition();
    MoveProjectView(from + wxPoint(10,0)); // Start drag
    static const int DRAGSTEPS = 5; // Use a higher number to see the drag in small steps. NOTE: Too small number causes drop in wrong position!
    for (int i = DRAGSTEPS; i > 0; --i)
    {
        wxPoint p(fromAbs.x + (toAbs.x - fromAbs.x) / i, fromAbs.y + (toAbs.y - fromAbs.y) / i);
        MoveOnScreen(p);
        //waitForIdle();
    }
    //    pause();
    wxUIActionSimulator().MouseUp();
    waitForIdle();
}

} // namespace