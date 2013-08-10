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

#include "HelperWatcher.h"

#include "UtilLog.h"
#include "UtilThread.h"
#include "Watcher.h"

namespace test {

void ASSERT_WATCHED_PATHS_COUNT(int n)
{
    util::thread::RunInMainAndWait([n] 
    { 
        ASSERT_EQUALS(gui::Watcher::get().getWatchedPathsCount(),n); 
    });
}

} // namespace