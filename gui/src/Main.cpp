#include "Main.h"

#include <wx/msgdlg.h>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/make_shared.hpp>
#include <boost/filesystem/operations.hpp>
#include "UtilLog.h"
#include "UtilLogAvcodec.h"
#include "Options.h"
#include "Window.h"
#include "Layout.h"
#include "FSWatcher.h"
#include "DebugReport.h"

/// \todo Fix auto-import warning, see http://gnuwin32.sourceforge.net/compile.html (auto import)

namespace gui {

IMPLEMENT_APP(Main)

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Main::Main()
:   wxApp()
{
#ifdef CATCH_ALL_ERRORS
    wxHandleFatalExceptions();
#endif // CATCH_ALL_ERRORS
}

Main::~Main()
{
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

bool Main::OnInit()
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
    Options::init();

    // The fonts cannot be initialized similar to pens and brushes
    // (leads to uninitialized wxStockGDI)
    Layout::initializeFonts();

    Log::Init();

	LOG_INFO << "Start";

    SetTopWindow(new Window());
    dynamic_cast<Window*>(GetTopWindow())->init();

    return true;
}

int Main::OnRun()
{
    //int j = 8; j = 0; int i = 6/j; // Uncomment for testing OnFatalException()
    //wxArrayString arr;arr[0];      // Uncomment for testing OnAssertFailure()
    //throw 4;                         // Uncomment for testing OnUnhandledException() directly (without going via OnExceptionInMainLoop())

    wxApp::OnRun(); // Make exception in this call for testing OnExceptionInMainLoop() - Typically, normal code of app.
    return 0;
}


int Main::OnExit()
{
    LOG_INFO;
    //Not: Log::Terminate() - OnUnhandledException() is called after leaving this method, and uses the log methods.
    return wxApp::OnExit();
}

#ifdef CATCH_ALL_ERRORS

void Main::OnAssertFailure(const wxChar *file, int Line, const wxChar *func, const wxChar *cond, const wxChar *msg)
{
    wxString File(file);
    wxString Function(func);
    wxString Condition(cond);
    wxString Message(msg);
    VAR_ERROR(File)(Line)(Function)(Condition)(Message);
    DebugReport::generate(ReportWxwidgetsAssertionFailure);
    wxApp::OnAssertFailure(file, Line, func, cond, msg);
}

bool Main::OnExceptionInMainLoop()
{
    // Rethrown in order to be handled in 'OnUnhandledException()'
    throw;
    return true;
}

void Main::OnUnhandledException()
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

void Main::OnFatalException()
{
    LOG_ERROR << std::endl;
    DebugReport::generate(ReportFatalException);
}

#endif // CATCH_ALL_ERRORS

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Main::serialize(Archive & ar, const unsigned int version)
{
    ar & *(dynamic_cast<Window*>(GetTopWindow()));
}
template void Main::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Main::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} // namespace
