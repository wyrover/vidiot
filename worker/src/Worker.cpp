// Copyright 2013-2016 Eric Raijmakers.
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

#include "StatusBar.h"
#include "UtilThread.h"
#include "WorkerEvent.h"

namespace worker {

static const unsigned int sMaximumBufferedWork = 1000;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Worker::Worker(const char* name, bool progress)
    : wxEvtHandler()
    , mName(name)
    , mVisibleProgress(progress)
    , mEnabled(true)
    , mRunning(false)
    , mPending(0)
    , mFifo(sMaximumBufferedWork)
    , mExecuted(0)
    , mExecutedLimit(0)
{
}

void Worker::start()
{
    mEnabled = true;
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
        mThread.reset();
    }
    QueueEvent(new WorkerQueueSizeEvent(0));
    if (mVisibleProgress)
    {
        gui::StatusBar::get().setProcessingText("");
        gui::StatusBar::get().hideProgressBar();
    }
    mFifo.flush();
}

bool Worker::isActive() const
{
    return mPending > 0;
}

//////////////////////////////////////////////////////////////////////////
// NEW WORK
//////////////////////////////////////////////////////////////////////////

void Worker::schedule(const WorkPtr& work)
{
    {
        boost::mutex::scoped_lock lock(mMutex);
        if (!mEnabled)
        {
            return;
        }
    }
    ++mPending;
    if (!mVisibleProgress)
    {
        work->stopShowingProgress();
    }
    mFifo.push(work);
    QueueEvent(new WorkerQueueSizeEvent(mFifo.getSize()));
    boost::mutex::scoped_lock lock(mMutex);
    if (mThread == nullptr)
    {
        mThread.reset(new boost::thread(std::bind(&Worker::thread,this))); // Only start this extra thread if there's actual work. Easier debugging.
    }
}

//////////////////////////////////////////////////////////////////////////
// WAIT FOR WORK ITEMS EXECUTED
//////////////////////////////////////////////////////////////////////////

void Worker::setExpectedWork(int expected)
{
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
    mRunning = true;
    VAR_INFO(this);
    util::thread::setCurrentThreadName(mName);
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
            bool done{ w->execute(mVisibleProgress) };
            util::thread::setCurrentThreadName(mName);
            w.reset(); // Clear, so that unfreezing is done if needed
            {
                boost::mutex::scoped_lock lock(mMutex);
                mCurrent.reset();
            }
            if (done)
            {
                mExecuted++;
                if (mExecuted == mExecutedLimit)
                {
                    mCondition.notify_all();
                }
            }
        }
        --mPending;
    }
    mRunning = false;
}

} //namespace
