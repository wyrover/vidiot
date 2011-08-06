#include "FixtureApplication.h"

#include "Application.h"
#include "UtilLog.h"
#include "Window.h"

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
    gui::Window::get().GetEventHandler()->QueueEvent(new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED,wxID_EXIT));

    // The exit causes the event loop to be activated again, resulting in one
    // extra call to onEventLoopEnter. That must be unblocked.
    mBarrierStarted.wait();

    // Wait until main thread 'OnRun' stopped
    mBarrierStopped.wait();
    return true;
}

void FixtureGui::start()
{
    // Do not define in header file. Then it will be optimized away also 
    // (at least on Visual Studio 2010).
}

//////////////////////////////////////////////////////////////////////////
// IEventLoopListener
//////////////////////////////////////////////////////////////////////////

void FixtureGui::onEventLoopEnter()
{
    mBarrierStarted.wait();
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