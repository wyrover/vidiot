#include "FixtureGui.h"

#include <wx/evtloop.h> 
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include "Application.h"
#include "AutoFolder.h"
#include "File.h"
#include "Project.h"
#include "ProjectView.h"
#include "Sequence.h"
#include "Window.h"
#include "UtilDialog.h"
#include "UtilLog.h"
#include "ids.h"

namespace test {

//static 
FixtureGui sInstance;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

FixtureGui::FixtureGui()
    :   mEnd(false)
    ,   mBarrierStart(2)
    ,   mBarrierStarted(2)
    ,   mBarrierStopped(2)
{
}

FixtureGui::~FixtureGui()
{
}

bool FixtureGui::setUpWorld()
{
    mThread.reset(new boost::thread(boost::bind(&FixtureGui::mainThread,this)));
    return true;
}

bool FixtureGui::tearDownWorld()
{
    ASSERT(!mEnd);
    mEnd = true;
    mBarrierStart.wait();
    if (mThread)
    {
        mThread->join();
    }
    return true;
}

bool FixtureGui::setUp()
{
    mBarrierStart.wait();
    mBarrierStarted.wait();
    return true;
}

bool FixtureGui::tearDown()
{
    wxDocument* doc = gui::Window::get().GetDocumentManager()->GetCurrentDocument();
    if (doc)
    {
        doc->Modify(false); // Avoid "Save yes/no/Cancel" dialog
    }
    // Must be done via an Event, since all wxWindows derived classes must be 
    // destroyed in the same thread as in which they were created, which is 
    // the main wxWidgets event thread.
    FixtureGui::triggerMenu(wxID_EXIT);

    // The exit causes the event loop to be activated again, resulting in one
    // extra call to onEventLoopEnter. That must be unblocked.
    mBarrierStarted.wait();

    // Wait until main thread 'OnRun' stopped
    mBarrierStopped.wait();
    return true;
}

//////////////////////////////////////////////////////////////////////////
// IEventLoopListener
//////////////////////////////////////////////////////////////////////////

void FixtureGui::onEventLoopEnter()
{
    mBarrierStarted.wait();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

// static
void FixtureGui::triggerMenu(int id)
{
    triggerMenu(gui::Window::get(), id);
}

// static
void FixtureGui::triggerMenu(wxWindow& window, int id)
{
    window.GetEventHandler()->QueueEvent(new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED,id));
}

// static
void FixtureGui::waitForIdle()
{
    static_cast<gui::Application*>(wxTheApp)->waitForIdle();
}

// static
model::FolderPtr FixtureGui::createProject()
{
    waitForIdle();
    triggerMenu(wxID_NEW);
    waitForIdle();
    return getRoot();
}

// static
model::FolderPtr FixtureGui::getRoot()
{
    model::FolderPtr root = model::Project::get().getRoot();
    return boost::static_pointer_cast<model::Folder>(root);
}

// static
model::FolderPtr FixtureGui::addAutoFolder( wxString path, model::FolderPtr parent )
{
    waitForIdle();
    gui::ProjectView::get().select(boost::assign::list_of(parent));
    waitForIdle();
    UtilDialog::setDir( path );
    triggerMenu(gui::ProjectView::get(),meID_NEW_AUTOFOLDER);
    waitForIdle();

    model::ProjectViewPtrs nodes = getRoot()->find( path );
    ASSERT(nodes.size() == 1);
    model::ProjectViewPtr node = nodes.front();
    ASSERT(node->isA<model::AutoFolder>())(node);
    model::FolderPtr folder = boost::static_pointer_cast<model::Folder>(node);
    return folder;
}

// static
model::FolderPtr FixtureGui::addFolder( wxString name, model::FolderPtr parent )
{
    waitForIdle();
    gui::ProjectView::get().select(boost::assign::list_of(parent));
    UtilDialog::setText( name );
    triggerMenu(gui::ProjectView::get(),meID_NEW_FOLDER);
    waitForIdle();

    model::ProjectViewPtrs nodes = getRoot()->find( name );
    ASSERT(nodes.size() == 1);
    model::ProjectViewPtr node = nodes.front();
    ASSERT(node->isA<model::Folder>())(node);
    model::FolderPtr folder = boost::static_pointer_cast<model::Folder>(node);
    return folder;
}

// static
model::SequencePtr FixtureGui::addSequence( wxString name, model::FolderPtr parent )
{
    waitForIdle();
    gui::ProjectView::get().select(boost::assign::list_of(parent));
    UtilDialog::setText( name );
    triggerMenu(gui::ProjectView::get(),meID_NEW_SEQUENCE);
    waitForIdle();

    model::ProjectViewPtrs nodes = getRoot()->find( name );
    ASSERT(nodes.size() == 1);
    model::ProjectViewPtr node = nodes.front();
    ASSERT(node->isA<model::Sequence>())(node);
    model::SequencePtr sequence = boost::static_pointer_cast<model::Sequence>(node);
    return sequence;
}

// static 
model::SequencePtr FixtureGui::createSequence( model::FolderPtr folder )
{
    waitForIdle();
    gui::ProjectView::get().select(boost::assign::list_of(folder));
    waitForIdle();
    triggerMenu(gui::ProjectView::get(),meID_CREATE_SEQUENCE);
    waitForIdle();

    model::ProjectViewPtrs nodes = getRoot()->find( folder->getName() );
    ASSERT(nodes.size() == 2); // The sequence and the folder
    model::SequencePtr result;
    BOOST_FOREACH( model::ProjectViewPtr node, nodes )
    {
        if (node->isA<model::Sequence>())
        {
            result = boost::static_pointer_cast<model::Sequence>(node);
        }
    }
    ASSERT(result);
    return result;
}

// static
model::Files FixtureGui::addFiles( std::list<wxString> paths, model::FolderPtr parent )
{
    waitForIdle();
    gui::ProjectView::get().select(boost::assign::list_of(parent));
    UtilDialog::setFiles( paths );
    triggerMenu(gui::ProjectView::get(),meID_NEW_FILE);
    waitForIdle();

    model::Files result;
    BOOST_FOREACH( wxString path, paths )
    {
        model::ProjectViewPtrs nodes = parent->find( path );
        ASSERT(nodes.size() == 1);
        model::ProjectViewPtr node = nodes.front();
        ASSERT(node->isA<model::File>())(node);
        result.push_back(boost::static_pointer_cast<model::File>(node));
    }
    return result;
}

// static
void FixtureGui::remove( model::ProjectViewPtr node )
{
    waitForIdle();
    gui::ProjectView::get().select(boost::assign::list_of(node));
    waitForIdle();
    triggerMenu(gui::ProjectView::get(),wxID_DELETE);
    waitForIdle();
}


// static 
int FixtureGui::countProjectView()
{
    FixtureGui::waitForIdle();
    gui::ProjectView::get().selectAll();
    model::ProjectViewPtrs selection = gui::ProjectView::get().getSelection();
    return selection.size();
}

//////////////////////////////////////////////////////////////////////////
// MAIN WXWIDGETS THREAD
//////////////////////////////////////////////////////////////////////////


void FixtureGui::mainThread()
{
    wxApp::SetInstance(new gui::Application(this));
    int argc = 1;
    char* argv = _strdup(gui::Application::sTestApplicationName);
    wxEntryStart(argc, &argv);

    while (!mEnd)
    {
        mBarrierStart.wait();
        if (!mEnd)
        {
            wxTheApp->CallOnInit();
            wxTheApp->OnRun();
            mBarrierStopped.wait();
        }
    }

    wxEntryCleanup();
}

} // namespace