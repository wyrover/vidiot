#include "FixtureGui.h"

#include <wx/evtloop.h> 
#include "Application.h"
#include "Window.h"
#include "UtilLog.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

//    bool wxEntryStart(int& argc, wxChar **argv)
//        instead of wxEntry. It doesn't call your app's OnInit() or run the main loop.
//        You can call wxTheApp->CallOnInit() to invoke OnInit() when needed in your tests.
//        You'll need to use
//        void wxEntryCleanup()

FixtureGui::FixtureGui()
{
    boost::mutex::scoped_lock lock(mMutex);
    mThread.reset(new boost::thread(boost::bind(&FixtureGui::thread,this)));
    mCondition.wait(lock);
}

FixtureGui::~FixtureGui()
{
    wxTheApp->OnExit();
    wxEntryCleanup();
    if (mThread)
    {
        mThread->join();
    }
}

//////////////////////////////////////////////////////////////////////////
// IEventLoopListener
//////////////////////////////////////////////////////////////////////////

void FixtureGui::onEventLoopEnter()
{
    mCondition.notify_all();
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

//////////////////////////////////////////////////////////////////////////
// WXWIDGETS MAIN THREAD
//////////////////////////////////////////////////////////////////////////

void FixtureGui::thread()
{
    {
        boost::mutex::scoped_lock lock(mMutex);
        wxApp::SetInstance(new gui::Application(this));
        int argc = 1;
        char* argv = _strdup(gui::Application::sTestApplicationName);
        wxEntryStart(argc, &argv);
        wxTheApp->OnInit();
    }
    wxTheApp->OnRun();
}

} // namespace