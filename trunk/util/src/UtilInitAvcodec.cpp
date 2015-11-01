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

#include "UtilInitAvcodec.h"

#include "Config.h"
#include "UtilEnum.h"
#include "UtilLog.h"
#include "UtilLogAvcodec.h"
#include "UtilMap.h"

std::map<int, wxString> Avcodec::mapAvcodecLevels = {
    { AV_LOG_QUIET, _("None") },
    { AV_LOG_FATAL, _("Fatal") },
    { AV_LOG_ERROR, _("Error") },
    { AV_LOG_WARNING, _("Warning") },
    { AV_LOG_INFO, _("Info") },
    { AV_LOG_VERBOSE, _("Verbose") },
};

//////////////////////////////////////////////////////////////////////////
// MEMBERS
//////////////////////////////////////////////////////////////////////////

const int Avcodec::sMaxLogSize = 500;
char* Avcodec::sFixedBuffer = 0;
int Avcodec::sLevel = AV_LOG_FATAL;
wxString Avcodec::sMostRecentLogLine("Increase the avcodec logging level in the .ini file to get detailed information.");

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

//static
int Avcodec::getDefaultLogLevel()
{
    return AV_LOG_ERROR;
}

//static
wxString Avcodec::getDefaultLogLevelString()
{
    return mapAvcodecLevels.find(AV_LOG_ERROR)->second;
}

//static
wxStrings Avcodec::getLogLevels()
{
	return UtilMap<int,wxString>(mapAvcodecLevels).values();
}

void Avcodec::configureLog()
{
	sLevel = UtilMap<int,wxString>(mapAvcodecLevels).reverseLookup(Config::ReadString(Config::sPathDebugLogLevelAvcodec),AV_LOG_ERROR);
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