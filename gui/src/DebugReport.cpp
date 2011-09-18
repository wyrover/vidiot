#include "DebugReport.h"

#include <wx/debugrpt.h>
#include <wx/confbase.h>
#include <wx/filename.h>
#include "UtilLog.h"
#include "Config.h"
#include "Options.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// GENERATE REPORT
//////////////////////////////////////////////////////////////////////////

// static
void DebugReport::generate(ReportType type)
{
    VAR_ERROR(type);
    wxDebugReport::Context ctx = wxDebugReport::Context_Current;
    if (type == ReportFatalException)
    {
        ctx = wxDebugReport::Context_Exception;
    }
    wxDebugReportCompress report;

    //report.AddAll(ctx);

    if (wxFileName(Config::getFileName()).FileExists())
    {
        report.AddFile(Config::getFileName(), wxT("options file"));
    }

    if (wxFileName(Log::getFileName()).FileExists())
    {
        Log::exit(); // NO MORE LOGGING BEYOND THIS POINT!!!
        report.AddFile(Log::getFileName(), wxT("text log file"));
    }

    if ( wxDebugReportPreviewStd().Show(report) )
    {
        report.Process();
    }
}

} // namespace
