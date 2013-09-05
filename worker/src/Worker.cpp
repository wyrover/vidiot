// Copyright 2013 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

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
,   mRunning(false)
,   mFifo(sMaximumBufferedWork)
,   mExecuted(0)
,   mExecutedLimit(0)
{
}

void Worker::abort()
{
    {
        boost::mutex::scoped_lock lock(mMutex);
        mEnabled = false;
        if (mCurrent)
        {
            mCurrent->abort();
            mCurrent.reset();
        }
        if (mRunning)
        {
            mFifo.flush();
            mFifo.push(WorkPtr()); // Unblock any pending 'pop'
        }
    }
    for (int i = 0; i < 100; ++i)
    {
        wxThread::This()->Sleep(100);
        {
            boost::mutex::scoped_lock lock(mMutex);
            if (!mRunning) { break; }
        }
    }
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
    {
        boost::mutex::scoped_lock lock(mMutex);
        if (!mEnabled)
        {
            return;
        }
    }
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
    {
        boost::mutex::scoped_lock lock(mMutex);
        mRunning = true;
    }
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
                    mRunning = false;
                    return;
                }
            }
        }
    }
    {
        boost::mutex::scoped_lock lock(mMutex);
        mRunning = false;
    }
}

} //namespace