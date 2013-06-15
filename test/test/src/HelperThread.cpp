#include "HelperThread.h"

#include "UtilEvent.h"
#include "UtilLog.h"

namespace test {

struct FunctionContainer
{
    FunctionContainer(Function function)
        : f(function)
    {}

    Function f;

    friend std::ostream& operator<<( std::ostream& os, const FunctionContainer& obj )
    {
        os << typeid(obj.f).name();
        return os;
    }
};

DECLARE_EVENT(EVENT_RUNINMAINTHREAD, EventRunInMainThread, FunctionContainer);
DEFINE_EVENT(EVENT_RUNINMAINTHREAD,  EventRunInMainThread, FunctionContainer);

struct RunInMainThreadHelper
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
{
    RunInMainThreadHelper(Function method)
        :   mDone(false)
    {
        ASSERT(!wxThread::IsMain());
        Bind( EVENT_RUNINMAINTHREAD, &RunInMainThreadHelper::onThreadEvent, this );
        QueueEvent(new EventRunInMainThread(FunctionContainer(method)));
        boost::mutex::scoped_lock lock(mMutex);
        while (!mDone)
        {
            mCondition.wait(lock);
        }
    }

    ~RunInMainThreadHelper()
    {
        Unbind( EVENT_RUNINMAINTHREAD, &RunInMainThreadHelper::onThreadEvent, this );
    }

    void onThreadEvent(EventRunInMainThread& event)
    {
        event.getValue().f();
        boost::mutex::scoped_lock lock(mMutex);
        mDone = true;
        mCondition.notify_all();
    }

    boost::condition_variable mCondition;
    boost::mutex mMutex;
    bool mDone;
};

void RunInMainThread(Function f)
{
    RunInMainThreadHelper run(f);
}

} // namespace