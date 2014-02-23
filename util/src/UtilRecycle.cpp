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

#include "UtilRecycle.h"

#ifdef _MSC_VER

// Source: http://forums.wxwidgets.org/viewtopic.php?t=26863=1
////////////////////////////////////////////////////////////////
// MSDN ? April 2001
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
// Compiles with Visual C++ 6.0. Runs on Windows 98 and probably Windows
// 2000 too.
#include <shellapi.h>
#include <windows.h>
#include <tchar.h>

#else // _MSC_VER
#endif

namespace util { namespace path {

#ifdef _MSC_VER

bool recycle(wxString file)
{
    SHFILEOPSTRUCT params;
    memset(&params,0,sizeof(SHFILEOPSTRUCT));
    params.fFlags |= FOF_SILENT;                // don't report progress
    params.fFlags |= FOF_NOERRORUI;             // don't report errors
    params.fFlags |= FOF_NOCONFIRMATION;        // don't confirm delete

    // Copy pathname to double-NULL-terminated string.
    TCHAR buf[_MAX_PATH + 1]; // allow one more character
    wcscpy(buf,file.c_str());
    buf[wcslen(buf)+1]=0;    // need two NULLs at end

    // Set SHFILEOPSTRUCT params for delete operation
    params.wFunc = FO_DELETE;                   // REQUIRED: delete operation
    params.pFrom = buf;                         // REQUIRED: which file(s)
    params.pTo = NULL;                          // MUST be NULL
    params.fFlags |= FOF_ALLOWUNDO;             // ..send to Recycle Bin
    return SHFileOperation(&params) == 0;       // do it!
}

#else // _MSC_VER
#endif

}} // namespace