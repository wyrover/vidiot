#include "GuiDebugReport.h"

#include <wx/debugrpt.h>
#include <wx/confbase.h>
#include "UtilLog.h"
#include "GuiOptions.h"

IMPLEMENTENUM(ReportType)

//////////////////////////////////////////////////////////////////////////
// GENERATE REPORT
//////////////////////////////////////////////////////////////////////////

// static
void GuiDebugReport::generate(ReportType type)
{
    VAR_ERROR(type);
    wxDebugReport::Context ctx = wxDebugReport::Context_Current;
    if (type == ReportFatalException)
    {
        ctx = wxDebugReport::Context_Exception;
    }
    wxDebugReportCompress report;

    report.AddAll(ctx);
    
    report.AddFile(GuiOptions::getOptionsFileName(), wxT("options file"));
    report.AddFile(GuiOptions::getLogFileName(), wxT("text log file"));

    if ( wxDebugReportPreviewStd().Show(report) )
    {
        if ( report.Process() )
        {
            LOG_INFO << "Report generated in '" <<  report.GetCompressedFileName().c_str() << "'";
        }
    }
}
