#include "UtilAssert.h"

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
    bool isDebuggerRunning = false;
#if (defined _MSC_VER) || (defined __BORLANDC__)
    isDebuggerRunning = wxIsDebuggerRunning();
#endif

    if (isDebuggerRunning)
    {
        Log::exit(); // Ensures that remaining log lines are flushed
#if (defined _MSC_VER) || (defined __BORLANDC__)
            __asm { int 3 };
#elif (defined __GNUC__) && (defined _DEBUG)
        __asm ("int $0x3");
#endif
    }
    else
    {
        sInstance->onAssert();
    }
}