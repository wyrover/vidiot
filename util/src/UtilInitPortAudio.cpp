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

#include "UtilInitPortAudio.h"

#include <pa_debugprint.h>
#include <portaudio.h>

#include "UtilLog.h"

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void PortAudio::init()
{
    PaUtil_SetDebugPrintFunction(log);
    PaError err = Pa_Initialize();
    ASSERT_EQUALS(err,paNoError)(Pa_GetErrorText(err));
}

void PortAudio::exit()
{
    PaError err = Pa_Terminate();
    ASSERT_EQUALS(err,paNoError)(Pa_GetErrorText(err));
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void PortAudio::log(const char * log)
{
    std::string s(log);
    std::string::iterator it = s.end() - 1;
    if (*it == '\n')
    {
         s.erase(it);
    }
    LOG_DEBUG << s;
}