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

#pragma once

#include "Test.h"

namespace test {

/// Construct this object before triggering an action that causes
/// work objects to be scheduled and executed. After the trigger
/// is given, call verify() to wait and verify that that exact
/// amount of Work objects was executed. To do the latter, a
/// limit is sent (on the Worker) on the maximum number of
/// executed Work.
struct ExpectExecutedWork
{
    explicit ExpectExecutedWork(int nWork, bool invisibleworker = false);
    virtual ~ExpectExecutedWork();

    /// Block execution until the given amount of Work has been
    /// scheduled AND executed by the Wworker.
    void wait();

private:

    int mWork;
    bool mWaited;
    bool mInvisibleWorker;

};

} // namespace
