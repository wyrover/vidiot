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
#include <wx/uri.h>
#endif

namespace util { namespace path {

#ifdef _MSC_VER

bool recycle(const wxString& file)
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

bool recycle(const wxString& file)
{
    // See: http://ubuntuforums.org/showthread.php?t=1766301
    bool success{false};

    wxFileName filename(file);
    ASSERT(!filename.IsDir())(filename); // Not supported

    wxString home(wxStandardPaths::Get().GetDocumentsDir());
    wxFileName recyclebin(home + "/.local/share/Trash", "");
    if (recyclebin.DirExists())
    {
        // Find proper name for .trashinfo file and for deleted file
        wxFileName infofile(recyclebin);
        infofile.AppendDir("info");
        infofile.SetName(filename.GetFullName());
        infofile.SetExt("trashinfo");
        int index = 2; // First 'renamed' file labeled with '2'.
        while (infofile.FileExists())
        {
            infofile.SetName(filename.GetFullName() + wxString::Format(wxT(".%d"), index++));
        }

        // Create .trashinfo file
        wxTextFile infofiletext(infofile.GetLongPath());
        success = infofiletext.Create(infofile.GetLongPath());
        if (success)
        {
            infofiletext.AddLine("[Trash Info]");
            infofiletext.AddLine("Path=" + wxURI(filename.GetLongPath()).BuildURI());
            infofiletext.AddLine("DeletionDate=" + wxDateTime::Now().Format("%FT%T"));
            success = infofiletext.Write();
            infofiletext.Close();
        }

        // Create copy of file in Trash
        if (success)
        {
            wxFileName trashedfile(recyclebin);
            trashedfile.AppendDir("files");
            trashedfile.SetFullName(infofile.GetFullName()); // Use the indexed name
            trashedfile.ClearExt(); // Remove the .trashinfo extension
            success = wxCopyFile(filename.GetLongPath(), trashedfile.GetLongPath());
        }

        // Remove file from original location
        if (success)
        {
            success = wxRemoveFile(filename.GetLongPath());
        }
    }
    else
    {
        success = wxRemoveFile(filename.GetLongPath());
    }
    return success;
}

#endif

}} // namespace
