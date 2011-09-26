#include "Application.h"

#include <wx/evtloop.h>
#include <wx/msgdlg.h>
#include <boost/exception/all.hpp>
#include <boost/filesystem/operations.hpp>
#include "Config.h"
#include "Dialog.h"
#include "IEventLoopListener.h"
#include "Layout.h"
#include "UtilInitAvcodec.h"
#include "UtilInitPortAudio.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
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
    // NOT: wxHandleFatalExceptions(); These are handled via the windows exception filter in Main.cpp
    wxHandleFatalExceptions(false);

    SetAppName(mEventLoopListener ? sTestApplicationName : "Vidiot");
    SetVendorName("Eric Raijmakers");

    // Logging initialization/termination is node made part of wxWidgets Init/Run/Exit
    // mechanism. Logging must be terminated as late as possible to avoid methods 
    // that log during shutdown to crash the shutdown process.
    // 
    // Typical example of that: logging the type of crash when generating a debugreport.
    Log::init(sTestApplicationName, GetAppName());
}

Application::~Application()
{
    Log::exit();
}

//////////////////////////////////////////////////////////////////////////
// IDLE HANDLING
//////////////////////////////////////////////////////////////////////////

void Application::waitForIdle()
{
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
    QueueEvent(new EventIdleTrigger(false));

    static int maxWaitTime = 4000;
    // timed_wait: To avoid indefinite waits. Not the best solution, but working...for now.
    // This caused problems, particularly with the Play() method of the HelperTimeline class.
    // Playback would continue indefinitely...
    mConditionIdle.timed_wait(lock, boost::posix_time::milliseconds(maxWaitTime));
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void Application::triggerIdle(EventIdleTrigger& event)
{
    boost::mutex::scoped_lock lock(mMutexIdle);
    Unbind(EVENT_IDLE_TRIGGER, &Application::triggerIdle, this);
    Bind(wxEVT_IDLE, &Application::onIdle, this);
    wxWakeUpIdle();
}
void Application::onIdle(wxIdleEvent& event)
{
    mConditionIdle.notify_all();
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

    // Can only be initialized after the logging has been initialized,
    // because it will log a lot during initialization.
    PortAudio::init();

    LOG_INFO;

    SetTopWindow(new Window());
    dynamic_cast<Window*>(GetTopWindow())->init();

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

    if (mEventLoopListener)
    {
        mEventLoopListener->onEventLoopEnter();
    }
}

int Application::OnExit()
{
    LOG_INFO;

    PortAudio::exit();
    Avcodec::exit();

    return wxApp::OnExit();
}

void Application::OnAssertFailure(const wxChar *file, int Line, const wxChar *function, const wxChar *condition, const wxChar *message)
{
    LOG_ERROR;
    wxString File(file);
    wxString Function(function);
    wxString Condition(condition);
    wxString Message(message);
    VAR_ERROR(File)(Line)(Function)(Condition)(Message);
    Dialog::get().getDebugReport(true, wxThread::IsMain());
}

bool Application::OnExceptionInMainLoop()
{
    LOG_ERROR;
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
    Dialog::get().getDebugReport();
    return true; // Continue the main loop, in order to be able to show the crash dialog
}

void Application::OnUnhandledException()
{
    LOG_ERROR;
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
    Dialog::get().getDebugReport();
}

void Application::OnFatalException()
{
    LOG_ERROR;
    Dialog::get().getDebugReport();
}

//////////////////////////////////////////////////////////////////////////
// IASSERT
//////////////////////////////////////////////////////////////////////////

void Application::onAssert()
{
    LOG_ERROR;
    Dialog::get().getDebugReport();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

bool Application::inTestMode() const
{
    return wxConfigBase::Get()->ReadBool(Config::sPathTest, false);
}

} // namespace
