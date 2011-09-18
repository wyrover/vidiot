#ifndef DEBUG_REPORT_H
#define DEBUG_REPORT_H

#include "UtilEnum.h"
#include "Dialog.h"

namespace gui {

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