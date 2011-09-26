#include "UtilThread.h"

#include <wx/thread.h>
#include "Dialog.h"
#include "UtilLog.h"
#include "Application.h"

namespace util {

// todo more use of this class

//////////////////////////////////////////////////////////////////////////
// HELPER CLASS
//////////////////////////////////////////////////////////////////////////

class NewThread : public wxThread
{
public:
    NewThread(Method threadmethod, bool joinable = false)
        :   wxThread(joinable ? wxTHREAD_JOINABLE : wxTHREAD_DETACHED)
        ,   mMethod(threadmethod)
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

SpawnThread::SpawnThread(Method threadmethod, bool joinable)
    :   mThread(new boost::thread(threadmethod))
//    :   mThread(new NewThread(threadmethod, joinable))
    ,   mJoinable(joinable)
{
    //wxThreadError result = wxTHREAD_NO_ERROR;
    //result = mThread->Create();
    //ASSERT_ZERO(result)(result); // Todo nice exception message
    //result = mThread->Run();
    //ASSERT_ZERO(result)(result); // Todo nice exception message
}

SpawnThread::~SpawnThread()
{
    if (mThread)
    {
        //mThread->join();
    }
    //if (mJoinable)
    //{
    //    mThread->Wait();
    //}
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

//wxThread& SpawnThread::getThread()
//{
//    return *mThread;
//}
//
} // namespace
