#include "UtilThread.h"

#include <wx/thread.h>
#include "Dialog.h"
#include "UtilLog.h"
#include "Application.h"

namespace util {

//////////////////////////////////////////////////////////////////////////
// HELPER CLASS
//////////////////////////////////////////////////////////////////////////

class NewThread : public wxThread
{
public:
    NewThread(Method threadmethod)
        :   mMethod(threadmethod)
    {
    }
    wxThread::ExitCode Entry()
    {
        mMethod();
        return static_cast<wxThread::ExitCode>(0); // success
    }
private:
    Method mMethod;
};

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

SpawnThread::SpawnThread(Method threadmethod)
{
    NewThread* thread = new NewThread(threadmethod);
    thread->Create();
    thread->Run();
}

} // namespace
