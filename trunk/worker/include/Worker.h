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