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

#include "UtilInitAvcodec.h"

#include "Config.h"
#include "UtilEnum.h"
#include "UtilLog.h"
#include "UtilLogAvcodec.h"

boost::bimap<int, wxString> Avcodec::mapAvcodecLevels = boost::assign::list_of<boost::bimap<int, wxString>::relation >
            (AV_LOG_QUIET,     _("None"))
            (AV_LOG_FATAL,     _("Fatal"))
            (AV_LOG_ERROR,     _("Error"))
            (AV_LOG_WARNING,   _("Warning"))
            (AV_LOG_INFO,      _("Info"))
            (AV_LOG_VERBOSE,   _("Verbose"));

//////////////////////////////////////////////////////////////////////////
// MEMBERS
//////////////////////////////////////////////////////////////////////////

const int Avcodec::sMaxLogSize = 500;
char* Avcodec::sFixedBuffer = 0;
int Avcodec::sLevel = AV_LOG_FATAL;
wxString Avcodec::sMostRecentLogLine(_("Increase the avcodec logging level in the .ini file to get detailed information."));

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
wxString Avcodec::getDefaultLogLevel()
{
    return mapAvcodecLevels.left.find(AV_LOG_ERROR)->second;
}

//static
std::list<wxString> Avcodec::getLogLevels()
{
    std::list<wxString> result;

    for ( auto value : mapAvcodecLevels.right )
    {
        result.push_back(value.first);
    }
    return result;
}

void Avcodec::configureLog()
{
    for ( auto value : mapAvcodecLevels.right )
    {
        if (value.first.IsSameAs(Config::ReadString(Config::sPathLogLevelAvcodec)))
        {
            sLevel = value.second;
            break;
        }
    }
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
        osComponent
            << "["
            << (*(AVClass**)ptr)->item_name(ptr)
            << ";"
            << (*(AVClass**)ptr)->class_name
            << "]";
    }

    sMostRecentLogLine = sFixedBuffer;
    Log().get("AVC    ") << osComponent.str() << " [" << sFixedBuffer << "]";
}