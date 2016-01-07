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

#include "UtilInitAvcodec.h"

#include "Config.h"
#include "UtilMap.h"

IMPLEMENTENUM(LogLevelAvcodec);

std::map<LogLevelAvcodec, wxString> LogLevelAvcodecConverter::getMapToHumanReadibleString()
{
    return
    {
        { LogLevelAvcodecQuiet, _("None") },
        { LogLevelAvcodecFatal, _("Fatal") },
        { LogLevelAvcodecError, _("Error") },
        { LogLevelAvcodecWarning, _("Warning") },
        { LogLevelAvcodecInfo, _("Info") },
        { LogLevelAvcodecVerbose, _("Verbose") },
    };
};

//////////////////////////////////////////////////////////////////////////
// MEMBERS
//////////////////////////////////////////////////////////////////////////

const int Avcodec::sMaxLogSize{ 500 };
char* Avcodec::sFixedBuffer{ 0 };
int Avcodec::sLevel{ AV_LOG_FATAL };
wxString Avcodec::sMostRecentLogLine{ "Increase the avcodec logging level in the .ini file to get detailed information." };

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void Avcodec::init()
{
    sFixedBuffer = new char[sMaxLogSize];
    av_register_all();
}

void Avcodec::exit()
{
    delete[] sFixedBuffer;
    sFixedBuffer = 0;
}

//////////////////////////////////////////////////////////////////////////
// LOCKING
//////////////////////////////////////////////////////////////////////////

boost::mutex Avcodec::sMutex;

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

void Avcodec::configureLog()
{
    static const std::map<LogLevelAvcodec, int> sMapEnumToAvLogLevel
    {
        { LogLevelAvcodecQuiet, AV_LOG_QUIET },
        { LogLevelAvcodecFatal, AV_LOG_FATAL },
        { LogLevelAvcodecError, AV_LOG_ERROR },
        { LogLevelAvcodecWarning, AV_LOG_WARNING },
        { LogLevelAvcodecInfo, AV_LOG_INFO },
        { LogLevelAvcodecVerbose, AV_LOG_VERBOSE },
    };

    LogLevelAvcodec level{ Config::ReadEnum<LogLevelAvcodec>(Config::sPathDebugLogLevelAvcodec) };
    ASSERT_MAP_CONTAINS(sMapEnumToAvLogLevel, level);
    sLevel = sMapEnumToAvLogLevel.find(level)->second;
    av_log_set_level(sLevel); // Only required for default avcodec log method
    av_log_set_callback(Avcodec::log);
}

wxString Avcodec::getMostRecentLogLine()
{
    return sMostRecentLogLine;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Avcodec::log(void *ptr, int level, const char * msg, va_list ap)
{
    if (level > sLevel) return;
    int len = vsnprintf(sFixedBuffer, sMaxLogSize, msg, ap);
    if ( len > 0 && sFixedBuffer[len-1] == '\n' )
    {
        // Strip new line in logged line
        sFixedBuffer[len-1] = '.';
    }

    std::ostringstream osComponent;
    if (ptr)
    {
        AVClass* info{ *(AVClass**)ptr };
        osComponent
            << "["
            << info->item_name(ptr)
            << ";"
            << info->class_name
            << "]";
    }

    sMostRecentLogLine = sFixedBuffer;
    Log().get("AVCODEC ") << osComponent.str() << " [" << sFixedBuffer << "]";
}