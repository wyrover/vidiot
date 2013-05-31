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
        :   mBarrier(2)
    {
        ASSERT(!wxThread::IsMain());
        Bind( EVENT_RUNINMAINTHREAD, &RunInMainThreadHelper::onThreadEvent, this );
        QueueEvent(new EventRunInMainThread(FunctionContainer(method)));
        mBarrier.wait();
    }

    ~RunInMainThreadHelper()
    {
        Unbind( EVENT_RUNINMAINTHREAD, &RunInMainThreadHelper::onThreadEvent, this );
    }

    void onThreadEvent(EventRunInMainThread& event)
    {
        event.getValue().f();
        mBarrier.wait();
    }

    boost::barrier mBarrier;
};

void RunInMainThread(Function f)
{
    RunInMainThreadHelper run(f);
}

} // namespace