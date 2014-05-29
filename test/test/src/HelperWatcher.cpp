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

#include "HelperWatcher.h"

#include "HelperThread.h"
#include "UtilLog.h"
#include "Watcher.h"

namespace test {

void ASSERT_WATCHED_PATHS_COUNT(int n)
{
    int currentWatchedPathsCount = -1; // Just: different from n
    RunInMainAndWait([&currentWatchedPathsCount] 
    { 
        currentWatchedPathsCount = gui::Watcher::get().getWatchedPathsCount();
    });
    ASSERT_EQUALS(currentWatchedPathsCount,n); // todo use this pattern everywhere: the assert must be in the test code. Then the break is on a more logical place on the call stack (easier analysis).
}

} // namespace