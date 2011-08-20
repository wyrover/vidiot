#include "Application.h"

#include <wx/msgdlg.h>
#include <boost/filesystem/operations.hpp>
#include <boost/exception/all.hpp>
#include "Config.h"
#include "DebugReport.h"
#include "IEventLoopListener.h"
#include "Layout.h"
#include "UtilLog.h"
#include "UtilLogAvcodec.h"
#include "UtilLogPortAudio.h"
#include "Window.h"

/// \TODO GCC Fix auto-import warning, see http://gnuwin32.sourceforge.net/compile.html (auto import)

namespace gui {

wxIMPLEMENT_APP_NO_MAIN(Application);

DEFINE_EVENT(EVENT_IDLE_TRIGGER,  EventIdleTrigger, bool);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

const wxString Application::sTestApplicationName = "VidiotTestSuite";

Application::Application(test::IEventLoopListener* eventLoopListener)
    :   wxApp()
    ,   IAssert()
    ,   mEventLoopListener(eventLoopListener)
{
#ifdef CATCH_ALL_ERRORS
    wxHandleFatalExceptions();
#endif // CATCH_ALL_ERRORS

    Bind(wxEVT_IDLE, &Application::onIdle, this);
    Bind(EVENT_IDLE_TRIGGER, &Application::triggerIdle, this);
}

Application::~Application()
{
    Unbind(wxEVT_IDLE, &Application::onIdle, this);
    Unbind(EVENT_IDLE_TRIGGER, &Application::triggerIdle, this);
}

//////////////////////////////////////////////////////////////////////////
// IDLE HANDLING
//////////////////////////////////////////////////////////////////////////

void Application::waitForIdle()
{
    boost::mutex::scoped_lock lock(mMutexIdle);
    QueueEvent(new EventIdleTrigger(false));
    mConditionIdle.wait(lock);
}


//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void Application::triggerIdle(EventIdleTrigger& event)
{
    boost::mutex::scoped_lock lock(mMutexIdle);
    wxWakeUpIdle();
}

void Application::onIdle(wxIdleEvent& event)
{
    mConditionIdle.notify_all();
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// GUI CALLBACKS
//////////////////////////////////////////////////////////////////////////

bool Application::OnInit()
{
    // Required for options.  The application name is explicitly not set.
    // This implies that the name of the executable is used. This allows for
    // running multiple instances with each its own configuration file.
    //SetAppName(_("Vidiot"));
    SetVendorName("Eric Raijmakers");

    // Done before options initialization
    // since after initializing the options,
    // the avcodec logging is initialized, which
    // in requires that avcodec is initialized.
    Avcodec::init();

    // Must be called before anything else,
    // since it distributes the initial options
    // which are used below.
    Config::init(GetAppName(), GetVendorName(), mEventLoopListener != 0);

    // The fonts cannot be initialized similar to pens and brushes
    // (leads to uninitialized wxStockGDI)
    Layout::initializeFonts();

    Avcodec::configureLog();
    if (inTestMode())
    {
        Log::SetReportingLevel(logDEBUG);
    }
    Log::Init();

    // Can only be initialized after the logging has been initialized,
    // because it will log a lot during initialization.
    PortAudio::init();

    LOG_INFO << "Start";

    SetTopWindow(new Window());
    dynamic_cast<Window*>(GetTopWindow())->init();

    return true;
}

int Application::OnRun()
{
    //int j = 8; j = 0; int i = 6/j; // todo make test for Uncomment for testing OnFatalException()
    //wxArrayString arr;arr[0];      // todo make test for Uncomment for testing OnAssertFailure()
    //throw 4;                       // todo make test for Uncomment for testing OnUnhandledException() directly (without going via OnExceptionInMainLoop())

    wxApp::OnRun(); // Make exception in this call for testing OnExceptionInMainLoop() - Typically, normal code of app.

    Log::Terminate();

    return 0;
}

void Application::OnEventLoopEnter(wxEventLoopBase* loop)
{
    if (mEventLoopListener)
    {
        mEventLoopListener->onEventLoopEnter();
    }
}

int Application::OnExit()
{
    //Not: LOG_INFO;// todo logging init _ terminate + startup/shutdown + exception handling
    //Not: Log::Terminate() - OnUnhandledException() is called after leaving this method, and uses the log methods.



    PortAudio::exit();
    Avcodec::exit();

    return wxApp::OnExit();
}

#ifdef CATCH_ALL_ERRORS

void Application::OnAssertFailure(const wxChar *file, int Line, const wxChar *func, const wxChar *cond, const wxChar *msg)
{
    wxString File(file);
    wxString Function(func);
    wxString Condition(cond);
    wxString Message(msg);
    VAR_ERROR(File)(Line)(Function)(Condition)(Message);
    DebugReport::generate(ReportWxwidgetsAssertionFailure);
    wxApp::OnAssertFailure(file, Line, func, cond, msg);
}

bool Application::OnExceptionInMainLoop()
{
    // Rethrown in order to be handled in 'OnUnhandledException()'
    throw;
    return true;
}

void Application::OnUnhandledException()
{
    try
    {
        throw;
    }
    catch (boost::exception &e)
    {
        LOG_ERROR << std::endl << "boost::exception" << std::endl << boost::diagnostic_information(e);
        DebugReport::generate(ReportBoostException);
    }
    catch (std::exception const& e)
    {
        LOG_ERROR << std::endl << "std::exception" << std::endl << boost::diagnostic_information(e);
        DebugReport::generate(ReportStdException);
    }
    catch ( ... )
    {
        LOG_ERROR << std::endl << "unknown exception type";
        DebugReport::generate(ReportUnhandledException);
    }
}

void Application::OnFatalException()
{
    LOG_ERROR << std::endl;
    DebugReport::generate(ReportFatalException);
}

#endif // CATCH_ALL_ERRORS

void Application::onAssert()
{
    DebugReport::generate(ReportAssertionFailure);
    exit(-1);
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

bool Application::inTestMode() const
{
    return wxConfigBase::Get()->ReadBool(Config::sPathTest, false);
}

} // namespace
