#include "Test.h"

#include <wx/evtloop.h> 
#include "Application.h"
#include "AProjectViewNode.h"
#include "ids.h"
#include "AutoFolder.h"
#include "UtilList.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "VideoFile.h"
#include "Window.h"
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


//////////////////////////////////////////////////////////////////////////
// HELPER CLASSES
//////////////////////////////////////////////////////////////////////////

DECLARE_EVENT(EVENT_QUEUE_MARKER, EventQueueMarker, bool)
DEFINE_EVENT(EVENT_QUEUE_MARKER, EventQueueMarker, bool)

    // wxWindow base class + constructor ares required for catching Idle events
class WaitForIdle : public wxWindow // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
{
public:
    WaitForIdle()
        :   wxWindow(&gui::Window::get(),wxID_ANY)
    {
        LOG_DEBUG;
        Bind(wxEVT_IDLE,    &WaitForIdle::onIdle,   this);
        gui::Window::get().Bind(EVENT_QUEUE_MARKER, &WaitForIdle::onEventQueueMarker, this);
        wait();
    }
    ~WaitForIdle()
    {
        Unbind(wxEVT_IDLE,  &WaitForIdle::onIdle,   this);
        gui::Window::get().Unbind(EVENT_QUEUE_MARKER, &WaitForIdle::onEventQueueMarker, this);
    }
    void onIdle(wxIdleEvent& event)
    {
        LOG_DEBUG << "Idle";
        conditionIdleSeen.notify_all();
        event.Skip();
    }
    void onEventQueueMarker(EventQueueMarker& event)
    {
        LOG_DEBUG << "EventQueueMarker";
        conditionMarkerSeen.notify_all();
        //wxWakeUpIdle();
        event.Skip();
    }
    void wait()
    {
        boost::mutex::scoped_lock lock(mMutex);

        //wxWakeUpIdle();
        LOG_DEBUG << "BEFORE WAKEUP";

        if (!wxTheApp->GetMainLoop()->Pending())
        {
            LOG_DEBUG << "NO WAIT";
            return;
        }

        wxTheApp->GetMainLoop()->WakeUpIdle();
        LOG_DEBUG << "BEFORE WAIT";

        //        gui::Window::get().QueueModelEvent(new EventQueueMarker(false));
        //      conditionMarkerSeen.wait(lock);
        conditionIdleSeen.wait(lock);
        LOG_DEBUG << "AFTER WAIT";

        //conditionIdleSeen.wait(lock);

    }
private:
    boost::mutex mMutex;
    boost::condition_variable conditionMarkerSeen;
    boost::condition_variable conditionIdleSeen;
};

class WaitForWindow : public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
{
public:
    WaitForWindow()
    {
        gui::Window::get().Bind(wxEVT_ACTIVATE,    &WaitForWindow::onActivate,   this);
        gui::Window::get().Bind(wxEVT_SET_FOCUS,    &WaitForWindow::onFocus,   this);
        gui::Window::get().Bind(wxEVT_KILL_FOCUS,    &WaitForWindow::onFocus,   this);
        gui::Window::get().Bind(wxEVT_CHILD_FOCUS,    &WaitForWindow::onChildFocus,   this);
        gui::Window::get().Bind(wxEVT_MOUSE_CAPTURE_LOST,    &WaitForWindow::onMouseCaptureLost,   this);
        gui::Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED,   &WaitForWindow::onHelp,             this, wxID_ABOUT);
    }
    ~WaitForWindow()
    {
    }
    void onActivate(wxActivateEvent& event)
    {
        LOG_DEBUG << "Focus";
        event.Skip();
    }
    void onFocus(wxFocusEvent& event)
    {
        LOG_DEBUG << "Focus";
        event.Skip();
    }
    void onChildFocus(wxChildFocusEvent& event)
    {
        LOG_DEBUG << "Focus";
        event.Skip();
    }
    void onMouseCaptureLost(wxMouseCaptureLostEvent& event)
    {
        LOG_DEBUG << "Focus";
        event.Skip();
    }
    void onHelp(wxCommandEvent& event)
    {
        NIY;
    }

};

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
    CxxTest::setAbortTestOnFail(false); 
    Log::setFileName("TestExample.log");
    Log::SetReportingLevel(logDEBUG);
    Log::Init();
}

ProjectViewTests::~ProjectViewTests()
{
    Log::Terminate();
}


//////////////////////////////////////////////////////////////////////////
// PER TEST INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void ProjectViewTests::setUp()
{

    boost::mutex::scoped_lock lock(mMutexMainThread);
    mMainThread.reset(new boost::thread(boost::bind(&ProjectViewTests::mainThread,this)));

    conditionMainThread.wait(lock);
}

void ProjectViewTests::mainThread()
{
    {
        boost::mutex::scoped_lock lock(mMutexMainThread);
        gui::Application* main = new gui::Application(this);
        wxApp::SetInstance(main);
        int argc = 1;
        char* argv = _strdup(gui::Application::sTestApplicationName);
        wxEntryStart(argc, &argv);
        wxTheApp->OnInit();
    }
    wxTheApp->OnRun();
}

void ProjectViewTests::OnEventLoopEnter()
{
    conditionMainThread.notify_all();
}


void ProjectViewTests::tearDown()
{
    wxTheApp->OnExit();
    wxEntryCleanup();
    if (mMainThread)
    {
        mMainThread->join();
    }
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

//    bool wxEntryStart(int& argc, wxChar **argv)
//        instead of wxEntry. It doesn't call your app's OnInit() or run the main loop.
//        You can call wxTheApp->CallOnInit() to invoke OnInit() when needed in your tests.
//        You'll need to use
//        void wxEntryCleanup()

void ProjectViewTests::testStartup()
{
    //gui::Application* main = new gui::Application();
    //wxApp::SetInstance(main);
    //int argc = 1;
    //char* argv = _strdup(gui::Application::sTestApplicationName);
    //wxEntryStart(argc, &argv);
    //wxTheApp->OnInit();

    //mThread.reset(new boost::thread(boost::bind(&ProjectViewTests::thread,this)));
    //wxTheApp->OnRun();

    //wxTheApp->OnExit();
    //wxEntryCleanup();

    //if (mThread)
    //{
    //    mThread->join();
    //}

    thread();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void ProjectViewTests::triggerMenu(int id)
{
    gui::Window::get().GetEventHandler()->QueueEvent(new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED,id));
}

void ProjectViewTests::triggerMenu(wxWindow& window, int id)
{
    window.GetEventHandler()->QueueEvent(new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED,id));
}

void ProjectViewTests::thread()
{

    // you can create top level-windows here or in OnInit(). Do your testing here
    //wxUIActionSimulator simu();

    WaitForIdle();
    triggerMenu(wxID_NEW);
    
    WaitForIdle();
    model::FolderPtr root = model::Project::get().getRoot();
    model::ProjectViewPtr rootNode = boost::static_pointer_cast<model::AProjectViewNode>(root);
        //    model::ProjectViewPtr rootNode = model::AProjectViewNode::Ptr(root->id());
    gui::ProjectView::get().select(boost::assign::list_of(rootNode));
        gui::Window::get().SetFocus();

    WaitForIdle();
    wxString sTestDir( "D:\\Vidiot\\test" );
    UtilDialog::setDir( sTestDir );
    triggerMenu(gui::ProjectView::get(),meID_NEW_AUTOFOLDER);

    WaitForIdle();
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

