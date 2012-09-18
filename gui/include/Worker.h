#ifndef WORKER_H
#define WORKER_H

#include "FifoWork.h"
#include "UtilEvent.h"
#include "UtilSingleInstance.h"

namespace gui {

DECLARE_EVENT(EVENT_WORKER_QUEUE_SIZE, WorkerQueueSizeEvent, long);
DECLARE_EVENT(EVENT_WORKER_EXECUTED_WORK, WorkerExecutedWorkEvent, long);

/// This class is responsible for running lengthy tasks in the
/// background.
class Worker
    :   public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    ,   public boost::noncopyable
    ,   public SingleInstance<Worker>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Worker();

    ~Worker();

    //////////////////////////////////////////////////////////////////////////
    // NEW WORK
    //////////////////////////////////////////////////////////////////////////

    void schedule(WorkPtr work);

    //////////////////////////////////////////////////////////////////////////
    // WAITING UNTIL WORK EXECUTED
    //////////////////////////////////////////////////////////////////////////

    void waitUntilWorkExecuted(); ///< Wait until one work item has been executed

private:

    bool mEnabled;
    boost::scoped_ptr<boost::thread> mThread;
    FifoWork mFifo;

    //////////////////////////////////////////////////////////////////////////
    // WAITING UNTIL WORK EXECUTED
    //////////////////////////////////////////////////////////////////////////

    boost::mutex mMutex;
    boost::condition_variable mCondition;

    //////////////////////////////////////////////////////////////////////////
    // THE THREAD
    //////////////////////////////////////////////////////////////////////////

    void thread();
};

} // namespace

#endif // WORKER_H