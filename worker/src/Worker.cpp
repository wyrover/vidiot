#include "Worker.h"

#include "WorkerEvent.h"

namespace worker {

static const unsigned int sMaximumBufferedWork = 1000;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Worker::Worker()
:   wxEvtHandler()
,   mEnabled(true)
,   mFifo(sMaximumBufferedWork)
,   mExecuted(0)
,   mExecutedLimit(0)
{
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
    boost::mutex::scoped_lock lock(mMutex);
    if (!mThread)
    {
        mThread.reset(new boost::thread(boost::bind(&Worker::thread,this))); // Only start this extra thread if there's actual work. Easier debugging.
    }
}

//////////////////////////////////////////////////////////////////////////
// WAIT FOR WORK ITEMS EXECUTED
//////////////////////////////////////////////////////////////////////////

void Worker::setExpectedWork(int expected)
{
    boost::mutex::scoped_lock lock(mMutex);
    mExecutedLimit = mExecuted + expected;
}

void Worker::waitForExecutionCount()
{
    boost::mutex::scoped_lock lock(mMutex);
    ASSERT_LESS_THAN_EQUALS(mExecuted,mExecutedLimit);
    while (mExecuted < mExecutedLimit)
    {
        mCondition.wait(lock);
    }
}

//////////////////////////////////////////////////////////////////////////
// THE THREAD
//////////////////////////////////////////////////////////////////////////

void Worker::thread()
{
    VAR_INFO(this);
    WorkPtr w;
    while (mEnabled)
    {
        w = mFifo.pop();
        QueueEvent(new WorkerQueueSizeEvent(mFifo.getSize()));

        if (w) // Check needed for the case that the fifo is aborted (and thus returns a 0 shared ptr)
        {
            w->execute();
            w.reset(); // Clear, so that unfreezing is done if needed
            {
                boost::mutex::scoped_lock lock(mMutex);
                mExecuted++;
                if (mExecuted == mExecutedLimit)
                {
                    mCondition.notify_all();
                }
                if (mFifo.getSize() == 0)
                {
                    mThread.reset();
                    return;
                }
            }
        }
    }
}

} //namespace