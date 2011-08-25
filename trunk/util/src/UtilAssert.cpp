#include "UtilAssert.h"

#include <wx/debug.h>
#include <wx/msgout.h> // For NIY
#include "UtilLog.h"

IAssert* sInstance;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

IAssert::IAssert()
{
    sInstance = this;
}

IAssert::~IAssert()
{
    sInstance = 0;
}

// static
void IAssert::breakIntoDebugger(const std::string& message)
{
#if (defined _MSC_VER) || (defined __BORLANDC__)
    if (wxIsDebuggerRunning())
    {
        Log::exit(); // Ensures that remaining log lines are flushed
        __asm { int 3 };
    }
#elif (defined __GNUC__) && (defined _DEBUG)
    __asm ("int $0x3");
#endif
    wxMessageOutputMessageBox().Printf("A fatal error was encountered:\n%s",message);
    sInstance->onAssert(); // TODO causes a hangup in module test, due to 'onEventLoopEnter' being triggered for the debugrpt window. In general, maybe hangups with module test and popup windows?
}
