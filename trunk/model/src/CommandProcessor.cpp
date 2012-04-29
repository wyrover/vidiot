#include "CommandProcessor.h"

#include "UtilLog.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

static CommandProcessor* sCurrent = 0;

CommandProcessor::CommandProcessor()
    :   wxCommandProcessor()
{
    sCurrent = this;
}

CommandProcessor::~CommandProcessor()
{
    sCurrent = 0;
}

// static
CommandProcessor& CommandProcessor::get()
{
    ASSERT(sCurrent);
    return *sCurrent;
}

} // namespace