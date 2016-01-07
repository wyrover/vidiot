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

#include "UtilAssert.h"

void breakIntoDebugger()
{
#ifdef _MSC_VER
    if (wxIsDebuggerRunning())
    {
        Log::exit(); // Ensures that remaining log lines are flushed
        __asm { int 3 };
    }
#else
#ifndef NDEBUG
    // On Linux, detection does not work, or interferes with generating the debug report.
    // For DEBUG builds do the interrupt, for RELEASE builds show the dialog.
     Log::exit(); // Ensures that remaining log lines are flushed
    __asm ("int $0x3");
#endif
    // Not reached if running in debugger
#endif
}
