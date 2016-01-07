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

void ASSERT_WATCHED_PATHS_COUNT(int n)
{
    WaitForIdle;
    int currentWatchedPathsCount = -1; // Just: different from n
    util::thread::RunInMainAndWait([&currentWatchedPathsCount]
    {
        currentWatchedPathsCount = model::FileWatcher::get().getWatchedPathsCount();
    });
    ASSERT_EQUALS(currentWatchedPathsCount,n);
}

} // namespace
