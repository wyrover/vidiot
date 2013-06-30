#ifndef WORKER_H
#define WORKER_H

#include "UtilSingleInstance.h"
#include "UtilFifo.h"
#include "Work.h"

namespace worker {

typedef Fifo<WorkPtr> FifoWork;

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
    void abort();
    ~Worker();

    //////////////////////////////////////////////////////////////////////////
    // NEW WORK
    //////////////////////////////////////////////////////////////////////////

    void schedule(WorkPtr work);

    //////////////////////////////////////////////////////////////////////////
    // WAIT FOR WORK ITEMS EXECUTED
    //////////////////////////////////////////////////////////////////////////

    void setExpectedWork(int expected);
    void waitForExecutionCount();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mEnabled;
    boost::scoped_ptr<boost::thread> mThread;
    FifoWork mFifo;
    WorkPtr mCurrent;

    int mExecuted;
    int mExecutedLimit;

    boost::mutex mMutex;
    boost::condition_variable mCondition;

    //////////////////////////////////////////////////////////////////////////
    // THE THREAD
    //////////////////////////////////////////////////////////////////////////

    void thread();
};

} // namespace

#endif // WORKER_H