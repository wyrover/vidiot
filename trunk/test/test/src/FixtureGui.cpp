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

//static
FixtureGui sInstance;

// static 
bool FixtureGui::UseRealUiEvents = false;;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

FixtureGui::FixtureGui()
    :   mEnd(false)
    ,   mStartingMainThread(false)
    ,   mBarrierConfigRead(2)
    ,   mBarrierStart(2)
    ,   mBarrierStarted(2)
    ,   mBarrierStopped(2)
    ,   mStartTime(0)
    ,   mHelperTestSuite(new HelperTestSuite())
{
}

FixtureGui::~FixtureGui()
{
}

bool FixtureGui::setUpWorld()
{
    util::thread::setCurrentThreadName("Test");
    mStartTime = time(0);
    try
    {
        mThread.reset(new boost::thread(boost::bind(&FixtureGui::mainThread,this)));
    }
    catch (boost::exception &e)
    {
        FATAL(boost::diagnostic_information(e));
    }

    mBarrierConfigRead.wait(); // When setUpWorld returns, the config must have been be read. Otherwise, setUp() below will use wrong config data.
    return true;
}

bool FixtureGui::tearDownWorld()
{
    ASSERT(!mEnd);
    {
        boost::mutex::scoped_lock lock(mEndMutex);
        mEnd = true;
        mBarrierStart.wait();
        // Release lock ensures main loop may inspect mEnd now
    }
    if (mThread)
    {
        mThread->join();
    }
    long runningtime = time(0) - mStartTime;
    std::ostringstream o;
    o << "Total running time: " << runningtime << " seconds (" << (runningtime/60) << "m" << (runningtime%60) << ")";
    wxMessageOutputDebug().Output(o.str());
    TS_TRACE(o.str().c_str());
    boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
    return true;
}

bool FixtureGui::setUp()
{
    if (!mHelperTestSuite->currentTestRequiresWindow()) { return true; } // Test was disabled or does not require window
     // Ensure that onEventLoopEnter blocks on mBarrierStarted. This blocking should
    // only be done for starting the main (application) event loop, not for any dialogs.
    mStartingMainThread = true;
    mBarrierStart.wait();
    mBarrierStarted.wait();
    VAR_DEBUG(this);
    return true;
}

bool FixtureGui::tearDown()
{
    mHelperTestSuite->testSuiteDone();
    if (!mHelperTestSuite->currentTestRequiresWindow()) { return true; } // Test was disabled or does not require window
    SetProjectUnmodified();

     // Ensure that onEventLoopEnter blocks on mBarrierStarted. This blocking should
    // only be done for (re)starting the main (application) event loop, not for any dialogs.
    mStartingMainThread = true;

    // Must be done via an Event, since all wxWindows derived classes must be
    // destroyed in the same thread as in which they were created, which is
    // the main wxWidgets event thread.
    WindowTriggerMenu(wxID_EXIT);

    // The exit causes the event loop to be activated again, resulting in one
    // extra call to onEventLoopEnter. That must be unblocked.
    mBarrierStarted.wait();

    // Wait until main thread 'OnRun' stopped
    mBarrierStopped.wait();
    VAR_DEBUG(this);
    return true;
}

void FixtureGui::start()
{
    // Do not define in header file. Then it will be optimized away
    // (at least on Visual Studio 2010).
}

//////////////////////////////////////////////////////////////////////////
// IEventLoopListener
//////////////////////////////////////////////////////////////////////////

void FixtureGui::onEventLoopEnter()
{
    if (mStartingMainThread)
    {
        // This check is needed to avoid hangups when opening new dialogs.
        // These will also cause the onEventLoopEnter event (since a modal
        // dialog can have its own event loop). In that case, there is only this
        // mBarrierStarted.wait() - the one in setup() is not called - it's
        // an immediate hangup.

        mBarrierStarted.wait();

        // Any subsequent 'onEventLoopEnter' will not block, except if that is indicated
        // specifically via mStartingMainThread
        mStartingMainThread = false;
    }
}

//////////////////////////////////////////////////////////////////////////
// MAIN WXWIDGETS THREAD
//////////////////////////////////////////////////////////////////////////

void FixtureGui::mainThread()
{
    util::thread::setCurrentThreadName("Main");
    wxApp::SetInstance(new gui::Application(this));

    FixtureGui::UseRealUiEvents = 
        wxStandardPaths::Get().GetExecutablePath().Contains("testui");

    int argc = 1;
#ifdef _MSC_VER
    char* argv = _strdup(gui::Application::sTestApplicationName);
    if (FixtureGui::UseRealUiEvents && (GetSystemMetrics( SM_REMOTESESSION ) != 0))
    {
        FATAL("Do not run in a remote desktop session (causes the generation of unwanted additional mouse events). Use VNC.");
    }
#else
    char* argv = strdup(gui::Application::sTestApplicationName);
    char *display = '\0';
    Display *mydisplay = XOpenDisplay(display);
    int myscreen = DefaultScreen(mydisplay);
    XSynchronize(mydisplay, 1);
    XInitThreads();
#endif
    wxEntryStart(argc, &argv);
	free(argv);

    mHelperTestSuite->readConfig();
    mBarrierConfigRead.wait();

    while (true)
    {
        mBarrierStart.wait();
        VAR_DEBUG(this);
        {
            boost::mutex::scoped_lock lock(mEndMutex);
            if (mEnd) break;
        }
        wxTheApp->CallOnInit();
        wxTheApp->OnRun();
        wxTheApp->OnExit();
        VAR_WARNING(this);
        mBarrierStopped.wait();
    }

    wxEntryCleanup();
}
} // namespace
