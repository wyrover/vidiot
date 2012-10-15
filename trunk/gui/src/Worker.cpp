#include "Worker.h"

namespace gui {

DEFINE_EVENT(EVENT_WORKER_QUEUE_SIZE, WorkerQueueSizeEvent, long);
DEFINE_EVENT(EVENT_WORKER_EXECUTED_WORK, WorkerExecutedWorkEvent, long);

static const unsigned int sMaximumBufferedWork = 1000;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Worker::Worker()
:   wxEvtHandler()
,   mEnabled(true)
,   mFifo(sMaximumBufferedWork)
{
    mThread.reset(new boost::thread(boost::bind(&Worker::thread,this)));
}

Worker::~Worker()
{
	mEnabled = false;
    mFifo.flush();
    mFifo.push(WorkPtr());
    if (mThread)
    {
        mThread->join();
    }
}

//////////////////////////////////////////////////////////////////////////
// NEW WORK
//////////////////////////////////////////////////////////////////////////

void Worker::schedule(WorkPtr work)
{
    mFifo.push(work);
    QueueEvent(new WorkerQueueSizeEvent(mFifo.getSize()));
}

//////////////////////////////////////////////////////////////////////////
// WAITING UNTIL WORK EXECUTED
//////////////////////////////////////////////////////////////////////////

void Worker::waitUntilQueueEmpty()
{
    boost::mutex::scoped_lock lock(mMutex);
    mCondition.wait(lock); // Ensure that the wait does not return before the first item is scheduled (and finished)
    while (mFifo.getSize() > 0)
    {
        mCondition.wait(lock);
    }
}

//////////////////////////////////////////////////////////////////////////
// THE THREAD
//////////////////////////////////////////////////////////////////////////

void Worker::thread()
{
    WorkPtr w;
    while (mEnabled)
    {
        w = mFifo.pop();
        QueueEvent(new WorkerQueueSizeEvent(mFifo.getSize()));

        if (w) // Check needed for the case that the fifo is aborted (and thus returns a 0 shared ptr)
        {
            w->execute();
            w.reset(); // Clear, so that unfreezing is done if needed
            boost::mutex::scoped_lock lock(mMutex);
            mCondition.notify_all();
        }
    }
}

} //namespace