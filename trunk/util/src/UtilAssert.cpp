#include "UtilAssert.h"

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
#ifdef _DEBUG
    #if (defined _MSC_VER) || (defined __BORLANDC__)
        __asm { int 3 };
    #elif defined(__GNUC__)
        __asm ("int $0x3");
    #else
    #  error Please supply instruction to break into code
    #endif
#else
    wxMessageOutputMessageBox().Printf("A fatal error was encountered:\n%s",message);
    sInstance->onAssert();
    Log::Terminate();
#endif
}
