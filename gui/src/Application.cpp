// Copyright 2013-2015 Eric Raijmakers.
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

#include "Application.h"

#include "CommandLine.h"
#include "Config.h"
#include "Dialog.h"
#include "Render.h"
#include "SubversionRevision.h"
#include "UtilInitAvcodec.h"
#include "UtilInitPortAudio.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "UtilStackWalker.h"
#include "UtilThread.h"
#include "VidiotVersion.h"
#include "Window.h"
#include <wx/cmdline.h>

//#include <vld.h> // Must be included in at least one cpp file

namespace gui {

wxIMPLEMENT_APP_NO_MAIN(Application);

DEFINE_EVENT(EVENT_IDLE_TRIGGER,  EventIdleTrigger, wxString);

struct wxLogImpl : public wxLog
{
#if WXWIN_COMPATIBILITY_2_8
    virtual void DoLog(wxLogLevel level, const wxChar *msg, time_t timestamp)
    {
        wxString wxMsg(msg);
        wxString wxLvl("wxLOG_LvlUnknown");
        switch (level)
        {
        case wxLOG_FatalError:    wxLvl = "wxLOG_FatalError"; break;
        case wxLOG_Error:         wxLvl = "wxLOG_Error"; break;
        case wxLOG_Warning:       wxLvl = "wxLOG_Warning"; break;
        case wxLOG_Message:       wxLvl = "wxLOG_Message"; break;
        case wxLOG_Status:        wxLvl = "wxLOG_Status"; break;
        case wxLOG_Info:          wxLvl = "wxLOG_Info"; break;
        case wxLOG_Debug:         wxLvl = "wxLOG_Debug"; break;
        case wxLOG_Trace:         wxLvl = "wxLOG_Trace"; break;
        }

        Log().get("WX      ") << wxLvl << ' ' << wxMsg;
    }
#endif // WXWIN_COMPATIBILITY_2_8

    virtual void DoLogRecord(wxLogLevel level, const wxString &msg, const wxLogRecordInfo &info) override
    {
        DoLogTextAtLevel(level,msg);
    }

    virtual void DoLogTextAtLevel(wxLogLevel level, const wxString &msg)
    {
        wxString wxMsg(msg);
        wxString wxLvl("wxLOG_LvlUnknown");
        switch (level)
        {
        case wxLOG_FatalError:    wxLvl = "wxLOG_FatalError"; break;
        case wxLOG_Error:         wxLvl = "wxLOG_Error"; break;
        case wxLOG_Warning:       wxLvl = "wxLOG_Warning"; break;
        case wxLOG_Message:       wxLvl = "wxLOG_Message"; break;
        case wxLOG_Status:        wxLvl = "wxLOG_Status"; break;
        case wxLOG_Info:          wxLvl = "wxLOG_Info"; break;
        case wxLOG_Debug:         wxLvl = "wxLOG_Debug"; break;
        case wxLOG_Trace:         wxLvl = "wxLOG_Trace"; break;
        }

        Log().get("WX      ") << wxLvl << ' ' << msg;
    }

    virtual void DoLogText (const wxString &msg)
    {
        Log().get("WX      ") << "wxLOG_Text"<< ' ' << msg;
    }
};

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

const wxString Application::sTestApplicationName = "VidiotTestSuite";

Application::Application(test::IEventLoopListener* eventLoopListener)
    :   wxApp()
    ,   IAssert()
    ,   mEventLoopListener(eventLoopListener)
    ,   mEventLoopStarted(false)
    ,   mCommandLine(boost::make_shared<CommandLine>())
{
    // NOT: wxHandleFatalExceptions(); These are handled via the windows exception filter in Main.cpp
    // todo for linux?
    wxHandleFatalExceptions(false);

    ::wxInitAllImageHandlers();

    SetAppName(mEventLoopListener ? sTestApplicationName : "Vidiot");
    SetVendorName("Eric Raijmakers");

    // Logging initialization/termination is not made part of wxWidgets Init/Run/Exit
    // mechanism. Logging must be terminated as late as possible to avoid methods
    // that log during shutdown to crash the shutdown process.
    //
    // Typical example of that: logging the type of crash when generating a debugreport.
    Log::init();


    LOG_ERROR << "Revision: " << SubversionRevision;
}

Application::~Application()
{
    Log::exit();
}

//////////////////////////////////////////////////////////////////////////
// IDLE HANDLING
//////////////////////////////////////////////////////////////////////////

void Application::WaitForIdleStart(wxString log)
{
    ASSERT(!wxThread::IsMain());
    // Original implementation:
    //    wxWakeUpIdle();
    //    mCondition.wait(lock);
    // However, it is possible that the idle event is received between these
    // two statements. This results in a (temporary) hangup of the tests. After
    // a while another idle event is received (guess), which resolves the hangup.
    // To avoid this, first an event is generated. This causes a method to be
    // called on the event loop. When wxWakeUpIdle() is called in that method,
    // the aforementioned interleaving problem cannot occur.

    boost::mutex::scoped_lock lock(mMutexIdle);

    // Queue a new event at the end of the queue. That ensures that any pending events
    // (including Idle events) are first handled. When this specific event is seen,
    // processing (for 'idle waiting') continues with method triggerIdle().
    Bind(EVENT_IDLE_TRIGGER, &Application::triggerIdle, this);
    QueueEvent(new EventIdleTrigger(log));

    static int maxWaitTime = 4000;
    // timed_wait: To avoid indefinite waits. Not the best solution, but working...for now.
    // This caused problems, particularly with the Play() method of the HelperTimeline class.
    // Playback would continue indefinitely...
    mConditionIdle.timed_wait(lock, boost::posix_time::milliseconds(maxWaitTime));
    LOG_DEBUG << "WAIT_IDLE_DONE " << log;
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

static int idlestart = 0;
static int idleend = 0;

void Application::triggerIdle(EventIdleTrigger& event)
{
    boost::mutex::scoped_lock lock(mMutexIdle);
    Unbind(EVENT_IDLE_TRIGGER, &Application::triggerIdle, this);
    Bind(wxEVT_IDLE, &Application::onIdle, this);
    LOG_DEBUG << "WAIT_IDLE " << idlestart << ' ' << event.getValue();
    idlestart++;
    wxWakeUpIdle();
}

void Application::onIdle(wxIdleEvent& event)
{
    boost::mutex::scoped_lock lock(mMutexIdle);
    mConditionIdle.notify_all();
    LOG_DEBUG << "WAIT_IDLE " << idleend;
    idleend++;
    Unbind(wxEVT_IDLE, &Application::onIdle, this);
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// GUI CALLBACKS
//////////////////////////////////////////////////////////////////////////

bool Application::OnInit()
{
    LOG_INFO;

    if ( !wxApp::OnInit() )
    {
        return false;
    }

    wxLog* previous = wxLog::SetActiveTarget(new wxLogImpl()); // Instantiated here. This ensures that wxWidgets messages are included in the log file and do not pop up new windows.
	if (previous)
	{
		delete previous; // Avoid memory leak during tests
	}

    // Done before options initialization since after initializing the options,
    // the avcodec logging is initialized, which requires that avcodec is initialized.
    Avcodec::init();

    // Must be called before anything else, since it distributes the initial options
    // which are used below.
    Config::init(GetAppName(), GetVendorName(), mEventLoopListener != 0);

    // Requires initialized Avcodec; thus, requires Avcodec::init().
    model::render::Render::initialize();

    // Can only be initialized after the logging has been initialized,
    // because it will log a lot during initialization.
    PortAudio::init();

    LOG_INFO;

    mEventLoopStarted = false;
    SetTopWindow(new Window());

    return true;
}

int Application::OnRun()
{
    LOG_INFO;

    wxApp::OnRun();

    return 0;
}

void Application::OnEventLoopEnter(wxEventLoopBase* loop)
{
    LOG_INFO;

    if (!mEventLoopStarted && loop->IsMain())
    {
        util::thread::setCurrentThreadName("Main");
        mEventLoopStarted = true;
        dynamic_cast<Window*>(GetTopWindow())->init();
    }

    if (mEventLoopListener)
    {
        mEventLoopListener->onEventLoopEnter();
    }
}

int Application::OnExit()
{
    LOG_INFO;

    boost::mutex::scoped_lock lock(mMutexIdle); // Wait until all idle handling done
    PortAudio::exit();
    Avcodec::exit();

    return wxApp::OnExit();
}

void Application::OnAssertFailure(const wxChar *file, int Line, const wxChar *function, const wxChar *condition, const wxChar *message)
{
    LOG_ERROR;
    LOG_STACKTRACE;
    wxString File(file);
    wxString Function(function);
    wxString Condition(condition);
    wxString Message(message);
    VAR_ERROR(File)(Line)(Function)(Condition)(Message);

    breakIntoDebugger();
    Dialog::get().getDebugReport(true, wxThread::IsMain());
}

bool Application::OnExceptionInMainLoop()
{
    LOG_ERROR;
    LOG_STACKTRACE;
    try
    {
        throw;
    }
    catch (boost::exception &e)
    {
        LOG_ERROR << "boost::exception" << std::endl << boost::diagnostic_information(e);
    }
    catch (std::exception const& e)
    {
        LOG_ERROR << "std::exception" << std::endl << boost::diagnostic_information(e);
    }
    catch ( ... )
    {
        LOG_ERROR << "unknown exception type";
    }
    breakIntoDebugger();
    Dialog::get().getDebugReport();
    return true; // Continue the main loop, in order to be able to show the crash dialog
}

void Application::OnUnhandledException()
{
    LOG_ERROR;
    LOG_STACKTRACE;
    try
    {
        throw;
    }
    catch (boost::exception &e)
    {
        LOG_ERROR << "boost::exception" << std::endl << boost::diagnostic_information(e);
        Dialog::get().getDebugReport();
    }
    catch (std::exception const& e)
    {
        LOG_ERROR << "std::exception" << std::endl << boost::diagnostic_information(e);
        Dialog::get().getDebugReport();
    }
    catch ( ... )
    {
        LOG_ERROR << "unknown exception type";
        Dialog::get().getDebugReport();
    }
    breakIntoDebugger();
    Dialog::get().getDebugReport();
}

void Application::OnFatalException()
{
    LOG_ERROR;
    LOG_STACKTRACE;
    breakIntoDebugger();
    Dialog::get().getDebugReport();
}

void Application::OnInitCmdLine (wxCmdLineParser &parser)
{
    static const wxCmdLineEntryDesc cmdLineDesc[] =
    {
        { wxCMD_LINE_SWITCH, "h", "help", "show help" },
        { wxCMD_LINE_SWITCH, "v", "verbose", "be verbose" }, // Required for running debug (apparently for logging)
        { wxCMD_LINE_OPTION, "e", "edit", "edit given file" },
        //{ wxCMD_LINE_PARAM, NULL, NULL, "input file", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_MULTIPLE },
        { wxCMD_LINE_NONE }
    };
    parser.SetDesc(cmdLineDesc);
}

bool Application::OnCmdLineParsed (wxCmdLineParser &parser)
{
    wxString filename;
    if (parser.Found("e", &filename))
    {
        CommandLine::get().EditFile = boost::optional<wxString>(filename);
    }
    return wxApp::OnCmdLineParsed(parser);
}

//////////////////////////////////////////////////////////////////////////
// IASSERT
//////////////////////////////////////////////////////////////////////////

void Application::onAssert()
{
    LOG_ERROR;
    LOG_STACKTRACE;
    Dialog::get().getDebugReport();
}

//////////////////////////////////////////////////////////////////////////
// VERSION INFORMATION
//////////////////////////////////////////////////////////////////////////

// static
wxString Application::getVersion()
{
    return VidiotVersion;
}

// static
int Application::getRevision()
{
    return SubversionRevision;
}

} // namespace
