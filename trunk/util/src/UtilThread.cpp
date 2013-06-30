#include "UtilThread.h"

#include "UtilEvent.h"

namespace util { namespace thread {

class RunInMainThread;
typedef boost::shared_ptr<RunInMainThread> RunInMainThreadPtr;
DECLARE_EVENT(EVENT_RUN_IN_MAIN_THREAD, EventRunInMainThread, RunInMainThreadPtr);
DEFINE_EVENT(EVENT_RUN_IN_MAIN_THREAD,  EventRunInMainThread, RunInMainThreadPtr);

class RunInMainThread
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    ,   public boost::enable_shared_from_this<RunInMainThread>
{
public:

    RunInMainThread(boost::function<void()> method, bool wait)
        : mMethod(method)
        , mWait(wait)
        , mDone(false)
    {
    }

    void run()
    {
        if (wxThread::IsMain())
        {
            mMethod();
        }
        else
        {
            Bind( EVENT_RUN_IN_MAIN_THREAD, &RunInMainThread::onThreadEvent, this );
            QueueEvent(new EventRunInMainThread(shared_from_this()));
            if (mWait)
            {
                boost::mutex::scoped_lock lock(mMutex);
                while (!mDone)
                {
                    mCondition.wait(lock);
                }
            }
        }
    }

    ~RunInMainThread()
    {
        if (mDone)
        {
            Unbind( EVENT_RUN_IN_MAIN_THREAD, &RunInMainThread::onThreadEvent, this );
        }
    }

    void onThreadEvent(EventRunInMainThread& event)
    {
        mMethod();
        event.getValue().reset();
        boost::mutex::scoped_lock lock(mMutex);
        mDone = true;
        mCondition.notify_all();
    }

private:

    boost::function<void()> mMethod;
    bool mWait;

    boost::condition_variable mCondition;
    boost::mutex mMutex;
    bool mDone;
};

void RunInMainAndWait(boost::function<void()> method)
{
    RunInMainThreadPtr obj = boost::make_shared<RunInMainThread>(method, true);
    obj->run();
}

void RunInMainAndDontWait(boost::function<void()> method)
{
    RunInMainThreadPtr obj = boost::make_shared<RunInMainThread>(method, false);
    obj->run();
}

void setCurrentThreadName(const char* name)
{
    bool isDebuggerRunning = false;
#if (defined _MSC_VER) || (defined __BORLANDC__)
    isDebuggerRunning = wxIsDebuggerRunning();
#endif

    if (isDebuggerRunning)
    {
        wxThreadIdType id = wxThread::GetCurrentId();

        const DWORD MS_VC_EXCEPTION=0x406D1388;
#pragma pack(push,8)
        typedef struct tagTHREADNAME_INFO
        {
            DWORD dwType; // Must be 0x1000.
            LPCSTR szName; // Pointer to name (in user addr space).
            DWORD dwThreadID; // Thread ID (-1=caller thread).
            DWORD dwFlags; // Reserved for future use, must be zero.
        } THREADNAME_INFO;
#pragma pack(pop)

        THREADNAME_INFO info;
        info.dwType = 0x1000;
        info.szName = name;
        info.dwThreadID = id;
        info.dwFlags = 0;

        __try
        {
            RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }
}

}} // namespace