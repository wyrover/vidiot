#ifndef DEBUG_REPORT_H
#define DEBUG_REPORT_H

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

class DebugReport
{
public:

    //////////////////////////////////////////////////////////////////////////
    // GENERATE REPORT
    //////////////////////////////////////////////////////////////////////////

    static void generate(ReportType type = ReportOther);
};

} // namespace

#endif // DEBUG_REPORT_H