#include "Worker.h"

#include "UtilThread.h"
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

void Worker::abort()
{
    mEnabled = false;
    {
        boost::mutex::scoped_lock lock(mMutex);
        if (mCurrent)
        {
            mCurrent->abort();
            mCurrent.reset();
        }
    }
    mFifo.flush();
    mFifo.push(WorkPtr());
    if (mThread)
    {
        mThread->join();
    }
}

Worker::~Worker()
{
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
    util::thread::setCurrentThreadName("Worker");
    while (mEnabled)
    {
        WorkPtr w = mFifo.pop();
        {
            boost::mutex::scoped_lock lock(mMutex);
            mCurrent = w;
        }
        QueueEvent(new WorkerQueueSizeEvent(mFifo.getSize()));
        if (w) // Check needed for the case that the fifo is aborted (and thus returns a 0 shared ptr)
        {
            w->execute();
            util::thread::setCurrentThreadName("Worker");
            {
                boost::mutex::scoped_lock lock(mMutex);
                w.reset(); // Clear, so that unfreezing is done if needed
                mCurrent.reset();
            }
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