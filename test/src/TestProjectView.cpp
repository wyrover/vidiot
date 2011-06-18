#include "TestProjectView.h"

#include <wx/evtloop.h> 
#include "Application.h"
#include "AProjectViewNode.h"
#include "ids.h"
#include "AutoFolder.h"
#include "UtilList.h"
#include "Window.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "VideoFile.h"
#include "ProjectView.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include "Project.h"
#include <boost/assign/list_of.hpp>
#include <boost/assign/list_of.hpp>
#include <list>
#include <wx/event.h>
#include <wx/uiaction.h>
#include "UtilEvent.h"
#include "UtilDialog.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// HELPER CLASSES
//////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

// static 
ProjectViewTests *ProjectViewTests::createSuite()
{ 
    return new ProjectViewTests(); 
}

// static 
void ProjectViewTests::destroySuite(ProjectViewTests *suite)
{
    delete suite; 
}

ProjectViewTests::ProjectViewTests()
{
}

ProjectViewTests::~ProjectViewTests()
{
}

//////////////////////////////////////////////////////////////////////////
// PER TEST INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void ProjectViewTests::setUp()
{
}

void ProjectViewTests::tearDown()
{
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void ProjectViewTests::testStartup()
{
    // you can create top level-windows here or in OnInit(). Do your testing here
    //wxUIActionSimulator simu();

    FixtureGui::waitForIdle();
    FixtureGui::triggerMenu(wxID_NEW);
    
    FixtureGui::waitForIdle();
    model::FolderPtr root = model::Project::get().getRoot();
    model::ProjectViewPtr rootNode = boost::static_pointer_cast<model::AProjectViewNode>(root);
    VAR_DEBUG(root->id())(rootNode->id());
    gui::ProjectView::get().select(boost::assign::list_of(rootNode));

    FixtureGui::waitForIdle();
    wxString sTestDir( "D:\\Vidiot\\test" );
    UtilDialog::setDir( sTestDir );
    FixtureGui::triggerMenu(gui::ProjectView::get(),meID_NEW_AUTOFOLDER);

    FixtureGui::waitForIdle();
    model::ProjectViewPtrs nodes = root->find("scene'20100102 12.32.48.avi");
    gui::ProjectView::get().selectAll();
    model::ProjectViewPtrs selection = gui::ProjectView::get().getSelection();
    model::ProjectViewPtrs files = model::AutoFolder::getSupportedFiles( boost::filesystem::path( sTestDir.fn_str() ) );
    ASSERT( selection.size() == files.size() + 1); // +1: Root

    //    gui::ProjectView::get().select(boost::assign::list_of(nodes.front()));

    //	gui::Window::get().QueueModelEvent(new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED,ID_OPTIONS));
    //	wxUIActionSimulator().MouseMove(100,100);
    //	wxUIActionSimulator().MouseClick();
}

} // namespace