#ifndef GUI_DEBUG_REPORT_H
#define GUI_DEBUG_REPORT_H

#include "UtilEnum.h"

namespace gui {

DECLAREENUM(ReportType, \
            ReportUnhandledException, \
            ReportFatalException, \
            ReportBoostException, \
            ReportStdException, \
            ReportAssertionFailure, \
            ReportWxwidgetsAssertionFailure, \
            ReportOther);

class GuiDebugReport
{
public:

    //////////////////////////////////////////////////////////////////////////
    // GENERATE REPORT
    //////////////////////////////////////////////////////////////////////////

    static void generate(ReportType type = ReportOther);
};

} // namespace

#endif // GUI_DEBUG_REPORT_H