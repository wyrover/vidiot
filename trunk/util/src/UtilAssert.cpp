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

#include "UtilAssert.h"

#include "UtilLog.h"

IAssert* sInstance;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

IAssert::IAssert()
{
    sInstance = this;
}

IAssert::~IAssert()
{
    sInstance = 0;
}

// static
void IAssert::breakIntoDebugger(const std::string& message)
{
    bool isDebuggerRunning = false;
#if (defined _MSC_VER) || (defined __BORLANDC__)
    isDebuggerRunning = wxIsDebuggerRunning();
#endif

    if (isDebuggerRunning)
    {
        Log::exit(); // Ensures that remaining log lines are flushed
#if (defined _MSC_VER) || (defined __BORLANDC__)
            __asm { int 3 };
#elif (defined __GNUC__) && (defined _DEBUG)
        __asm ("int $0x3");
#endif
    }
    else
    {
        sInstance->onAssert();
    }
}