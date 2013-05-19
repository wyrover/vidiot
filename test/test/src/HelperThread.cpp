#include "HelperThread.h"

#include "UtilEvent.h"
#include "UtilLog.h"

namespace test {

std::ostream& operator<<( std::ostream& os, const Function& obj )
{
    os << typeid(obj).name();
    return os;
}

DECLARE_EVENT(EVENT_RUNINMAINTHREAD, EventRunInMainThread, Function);
DEFINE_EVENT(EVENT_RUNINMAINTHREAD,  EventRunInMainThread, Function);

struct RunInMainThreadHelper
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
{
    RunInMainThreadHelper(Function method)
        :   mBarrier(2)
    {
        ASSERT(!wxThread::IsMain());
        Bind( EVENT_RUNINMAINTHREAD, &RunInMainThreadHelper::onThreadEvent, this );
        QueueEvent(new EventRunInMainThread(method));
        mBarrier.wait();
    }

    ~RunInMainThreadHelper()
    {
        Unbind( EVENT_RUNINMAINTHREAD, &RunInMainThreadHelper::onThreadEvent, this );
    }

    void onThreadEvent(EventRunInMainThread& event)
    {
        event.getValue()();
        mBarrier.wait();
    }

    boost::barrier mBarrier;
};

void RunInMainThread(Function f)
{
    RunInMainThreadHelper run(f);
}

} // namespace