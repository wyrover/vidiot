// Copyright 2013,2014 Eric Raijmakers.
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
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Worker(const char* name, bool progress);

    void start();

    /// Abort any pending work and avoid new work being scheduled
    void abort();

    virtual ~Worker();

    //////////////////////////////////////////////////////////////////////////
    // NEW WORK
    //////////////////////////////////////////////////////////////////////////

    void schedule(const WorkPtr& work);

    //////////////////////////////////////////////////////////////////////////
    // WAIT FOR WORK ITEMS EXECUTED
    //////////////////////////////////////////////////////////////////////////

    void setExpectedWork(int expected);
    void waitForExecutionCount();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    const char* mName;
    const bool mVisibleProgress;
    std::atomic<bool> mEnabled;
    std::atomic<bool> mRunning;
    boost::scoped_ptr<boost::thread> mThread;
    FifoWork mFifo;
    WorkPtr mCurrent;

    std::atomic<int> mExecuted;
    std::atomic<int> mExecutedLimit;

    boost::mutex mMutex;
    boost::condition_variable mCondition;

    //////////////////////////////////////////////////////////////////////////
    // THE THREAD
    //////////////////////////////////////////////////////////////////////////

    void thread();
};

/// Worker for work objects that show progress in the status bar
class VisibleWorker
    : public Worker
    ,   public SingleInstance<VisibleWorker>
{
public:
    VisibleWorker() : Worker("VisibleWorker", true) {}
};

/// Worker for background processing
class InvisibleWorker
    : public Worker
    ,   public SingleInstance<InvisibleWorker>
{
public:
    InvisibleWorker() : Worker("InvisibleWorker", false) {}
};

} // namespace

#endif
