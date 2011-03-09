#include "GuiMain.h"

#include <wx/msgdlg.h>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include "UtilLog.h"
#include "UtilLogAvcodec.h"
#include "GuiOptions.h"
#include "GuiWindow.h"
#include "Layout.h"
#include "GuiDebugReport.h"
#include "Project.h"

// TODO Fix auto-import warning, see http://gnuwin32.sourceforge.net/compile.html (auto import)

namespace gui {

IMPLEMENT_APP(GuiMain)

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

GuiMain::GuiMain()
:   mProject(0)
,   mDone(false)
{
#ifdef CATCH_ALL_ERRORS
    wxHandleFatalExceptions();
#endif // CATCH_ALL_ERRORS

    Bind(wxEVT_IDLE,                   &GuiMain::OnIdle,           this);
    Bind(model::EVENT_OPEN_PROJECT,    &GuiMain::OnOpenProject,    this);
    Bind(model::EVENT_CLOSE_PROJECT,   &GuiMain::OnCloseProject,   this);
}

GuiMain::~GuiMain()
{
    Unbind(wxEVT_IDLE,                   &GuiMain::OnIdle,           this);
    Unbind(model::EVENT_OPEN_PROJECT,    &GuiMain::OnOpenProject,    this);
    Unbind(model::EVENT_CLOSE_PROJECT,   &GuiMain::OnCloseProject,   this);
}

//////////////////////////////////////////////////////////////////////////
// PROJECT EVENTS
//////////////////////////////////////////////////////////////////////////

void GuiMain::OnOpenProject( model::EventOpenProject &event )
{
    mProject = event.getValue();
    event.Skip();
}

void GuiMain::OnCloseProject( model::EventCloseProject &event )
{
    mProject = 0;
    event.Skip();
}

model::Project* GuiMain::getProject() const
{
    return mProject;
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

bool GuiMain::OnInit()
{
    // Must be called asap to avoid cd'ing before this statement
    // first initializes the boost initial path variable.
    boost::filesystem::path initialpath = boost::filesystem::initial_path();

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
    GuiOptions::init();

    // The fonts cannot be initialized similar to pens and brushes
    // (leads to uninitialized wxStockGDI)
    Layout::initializeFonts();

    Log::Init();

	LOG_INFO << "Start";

    SetTopWindow(new GuiWindow());
    dynamic_cast<GuiWindow*>(GetTopWindow())->init();

    return true;
}

int GuiMain::OnRun()
{
    //int j = 8; j = 0; int i = 6/j; // Uncomment for testing OnFatalException()
    //wxArrayString arr;arr[0];      // Uncomment for testing OnAssertFailure()
    //throw 4;                         // Uncomment for testing OnUnhandledException() directly (without going via OnExceptionInMainLoop())

    wxApp::OnRun(); // Make exception in this call for testing OnExceptionInMainLoop() - Typically, normal code of app.
    return 0;
}


int GuiMain::OnExit()
{
    LOG_INFO;
    //Not: Log::Terminate() - OnUnhandledException() is called after leaving this method, and uses the log methods.
    return wxApp::OnExit();
}

#ifdef CATCH_ALL_ERRORS

void GuiMain::OnAssertFailure(const wxChar *file, int Line, const wxChar *func, const wxChar *cond, const wxChar *msg)
{
    wxString File(file);
    wxString Function(func);
    wxString Condition(cond);
    wxString Message(msg);
    VAR_ERROR(File)(Line)(Function)(Condition)(Message);
    GuiDebugReport::generate(ReportWxwidgetsAssertionFailure);
    wxApp::OnAssertFailure(file, Line, func, cond, msg);
}

bool GuiMain::OnExceptionInMainLoop()
{
    // Rethrown in order to be handled in 'OnUnhandledException()'
    throw;
    return true;
}

void GuiMain::OnUnhandledException()
{
    try
    {
        throw;
    }
    catch (boost::exception &e)
    {
        LOG_ERROR << std::endl << "boost::exception" << std::endl << boost::diagnostic_information(e);
        GuiDebugReport::generate(ReportBoostException);
    }
    catch (std::exception const& e)
    {
        LOG_ERROR << std::endl << "std::exception" << std::endl << boost::diagnostic_information(e);
        GuiDebugReport::generate(ReportStdException);
    }
    catch ( ... )
    {
        LOG_ERROR << std::endl << "unknown exception type";
        GuiDebugReport::generate(ReportUnhandledException);
    }
}

void GuiMain::OnFatalException()
{
    LOG_ERROR << std::endl;
    GuiDebugReport::generate(ReportFatalException);
}

#endif // CATCH_ALL_ERRORS

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void GuiMain::OnIdle(wxIdleEvent& event)
{
    // Do idle processing, ask for more idle
    // processing if we haven't finished the task
    if (!mDone)
        event.RequestMore();
    mDone = true;
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void GuiMain::serialize(Archive & ar, const unsigned int version)
{
    ar & *(dynamic_cast<GuiWindow*>(GetTopWindow()));
}
template void GuiMain::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void GuiMain::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} // namespace
