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

#include "Test.h"

namespace test {

ExpectExecutedWork::ExpectExecutedWork(int nWork, bool invisibleworker)
    : mWork(nWork)
    , mWaited(false)
    , mInvisibleWorker(invisibleworker)
{
    if (mInvisibleWorker)
    {
        worker::InvisibleWorker::get().setExpectedWork(mWork);
    }
    else
    {
        worker::VisibleWorker::get().setExpectedWork(mWork);
    }
}

ExpectExecutedWork::~ExpectExecutedWork()
{
    ASSERT(mWaited);
}

void ExpectExecutedWork::wait()
{
    ASSERT(!mWaited);
    mWaited = true;
    if (mInvisibleWorker)
    {
        worker::InvisibleWorker::get().waitForExecutionCount();
    }
    else
    {
        worker::VisibleWorker::get().waitForExecutionCount();
    }
    // Any work done generates at least one event (WorkDoneEvent).
    // Wait until those events have been handled by the main thread.
    WaitForIdle;
}

} // namespace
