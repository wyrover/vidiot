// Copyright 2013-2015 Eric Raijmakers.
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

#ifdef _MSC_VER
#include <pa_debugprint.h>
#endif
#include <portaudio.h>

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const PaHostApiTypeId& id)
{
    #define LOG_ENUM(enumvalue) case enumvalue: os << #enumvalue << '(' << static_cast<int>(enumvalue) << ')';  break
    switch (id)
    {
        LOG_ENUM(paInDevelopment);
        LOG_ENUM(paDirectSound);
        LOG_ENUM(paMME);
        LOG_ENUM(paSoundManager);
        LOG_ENUM(paCoreAudio);
        LOG_ENUM(paOSS);
        LOG_ENUM(paALSA);
        LOG_ENUM(paAL);
        LOG_ENUM(paBeOS);
        LOG_ENUM(paWDMKS);
        LOG_ENUM(paJACK);
        LOG_ENUM(paWASAPI);
        LOG_ENUM(paAudioScienceHPI);
        default: os << "Unknown" << '(' << static_cast<int>(id) << ')'; break;
    }
    return os;
}

void PortAudio::init()
{
#ifdef _MSC_VER
    PaUtil_SetDebugPrintFunction(log);
#endif
    PaError err = Pa_Initialize();
    ASSERT_EQUALS(err,paNoError)(Pa_GetErrorText(err));

    PaHostApiIndex count = Pa_GetHostApiCount();
    ASSERT_MORE_THAN_EQUALS_ZERO(count);

    PaHostApiIndex defaultIndex = Pa_GetDefaultHostApi();
    ASSERT_MORE_THAN_EQUALS_ZERO(defaultIndex);
    ASSERT_LESS_THAN(defaultIndex,count);

    const PaHostApiInfo * paInfo = Pa_GetHostApiInfo(defaultIndex);
    ASSERT_NONZERO(paInfo);

    LOG_INFO
        << "[Type=" << paInfo->type
        << "][Name=" << paInfo->name
        << "][DeviceCount=" << paInfo->deviceCount
        << "][DefaultInputDevice=" << paInfo->defaultInputDevice
        << "][DefaultOutputDevice=" << paInfo->defaultOutputDevice
        << "]";

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
