#include "TestTimelinesView.h"

#include <wx/evtloop.h> 
#include "Application.h"
#include "AProjectViewNode.h"
#include "ids.h"
#include "AutoFolder.h"
#include "Sequence.h"
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
#include "FixtureGui.h"

namespace test {

// TODO replace wxfilename with boost::filesystem everywhere
void TestTimelinesView::testTabs()
{
    //wxUIActionSimulator simu();
    //	wxUIActionSimulator().MouseMove(100,100);
    //	wxUIActionSimulator().MouseClick();

    boost::filesystem::path path( "D:\\Vidiot\\test" );
    wxString sFolder1( "Folder1" );
    wxString sSequence1( "Sequence1" );

    model::FolderPtr root = FixtureGui::createProject();
    model::FolderPtr autofolder = FixtureGui::addAutoFolder( path.generic_string() );

    gui::ProjectView::get().selectAll();
    FixtureGui::waitForIdle();
    model::ProjectViewPtrs selection = gui::ProjectView::get().getSelection();
    model::ProjectViewPtrs files = model::AutoFolder::getSupportedFiles( path );
    ASSERT( selection.size() == files.size() + 2); // +2: Root + Autofolder node
    
    model::FolderPtr folder1 = FixtureGui::addFolder( sFolder1 );
    model::SequencePtr sequence1 = FixtureGui::addSequence( sSequence1, folder1 );
    ASSERT( sequence1->getParent() == folder1 );

    gui::ProjectView::get().selectAll();
    FixtureGui::waitForIdle();
    selection = gui::ProjectView::get().getSelection();
    ASSERT( selection.size() == files.size() + 4); // +4: Root + Autofolder + Folder + Sequence node
}

} // namespace