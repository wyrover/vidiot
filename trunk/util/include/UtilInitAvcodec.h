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

#pragma once

#include <cstdarg> // va_list

struct AVRational;
struct AVCodecContext;
struct AVFormatContext;
struct AVStream;
struct AVPacket;
struct AVOutputFormat;

class Avcodec
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    static void init();
    static void exit();

    //////////////////////////////////////////////////////////////////////////
    // LOCKING
    //////////////////////////////////////////////////////////////////////////

    /// This mutex is needed to ensure that several non-thread-safe avcodec
    /// methods are never executed in parallel:
    /// - av_open_input_file
    /// - av_close_input_file
    /// - av_find_stream_info
    /// - avcodec_open
    /// - avcodec_close
    static boost::mutex sMutex;

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    static int getDefaultLogLevel();
    static wxString getDefaultLogLevelString();
    static wxStrings getLogLevels();
    static std::map<int, wxString> mapAvcodecLevels;
    static void configureLog();
    static wxString getMostRecentLogLine();

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    static const int sMaxLogSize;
    static char* sFixedBuffer;
    static int sLevel;
    static wxString sMostRecentLogLine;

private:

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    static void log(void *ptr, int level, const char * msg, va_list ap);
};
