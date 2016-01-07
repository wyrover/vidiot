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

namespace test {

#define WaitForIdle WaitForIdleImpl(__FILE__, __LINE__)

/// Wait until the main application loop signals Idle via a idle event.
/// If the loop is already idle, an extra idle event is triggered.
void WaitForIdleImpl(const char* p_szFileName, const size_t& p_lLine);

/// Delay the test for 60 seconds to allow using the GUI (debugging)
void pause(int ms = 60000);

} // namespace
