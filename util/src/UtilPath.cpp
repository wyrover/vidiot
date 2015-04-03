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

#include "UtilPath.h"

#include "Config.h"
#include "UtilLogWxwidgets.h"

namespace util { namespace path {

wxFileName normalize(wxFileName filename)
{
    bool normalizeResult = filename.Normalize();
    ASSERT(normalizeResult)(filename);
    return filename;
}

wxString toName(const wxFileName& filename)
{
    wxString result;
    if ( filename.IsDir() )
    {
        wxArrayString dirs = filename.GetDirs();
        if ( dirs.GetCount() > 0 )
        {
            result = dirs.Last();
        }
        else
        {
            result = filename.GetVolume() + wxFileName::GetVolumeSeparator();
        }
    }
    else
    {
        result = filename.GetFullName();
    }
    return result;
}

wxString toPath(const wxFileName& filename)
{
    wxString result = normalize(filename).GetFullPath();
    if ( wxEndsWithPathSeparator(result) )
    {
        result.erase(result.length() - 1);
    }
    return result;
}

wxFileName toFileName(const wxString& path)
{
    wxString corrected( path );
    if (wxDirExists( corrected ))
    {
        return wxFileName(corrected, "");
    }
    else
    {
        return wxFileName(corrected);
    }
}

time_t lastModifiedTime(const wxFileName& filename)
{
    // !dirname.empty() && dirname.Last() != wxT('\\'),
    time_t result = 0;
    if (filename.Exists())
    {
#ifdef _MSC_VER
        if (filename.IsDir() && filename.HasVolume() || filename.GetDirCount() == 0)
        {
            // Calling wxFileModificationTime or filename.GetModificationTime() with "C:\\" results in crash, use "C:" instead.
            result = wxFileModificationTime(filename.GetVolume() + ":");
            return result;
        }
#endif
        wxDateTime dt = filename.GetModificationTime();
        if (dt.IsValid())
        {
            result = dt.GetTicks();
        }
    }

    return result;
}

bool equals(const wxFileName& f1, const wxFileName& f2)
{
    return toPath( f1 ).IsSameAs( toPath( f2 ) );
}

bool equals(const wxString& f1, const wxFileName& f2)
{
    return equals( wxFileName( f1 ), f2 );
}

bool equals(const wxFileName& f1, const wxString& f2)
{
    return equals( f1, wxFileName( f2 ) );
}

bool equals(const wxString& f1, const wxString& f2)
{
    return equals( wxFileName( f1 ), wxFileName( f2 ) );
}

bool isParentOf(const wxFileName& parent, const wxFileName& child)
{
    wxString sChild = toPath( child );
    wxString sParent = toPath( parent );

    return !(sChild.IsSameAs(sParent)) && sChild.StartsWith( sParent );
}

bool isParentOf(const wxString& parent, const wxFileName& child)
{
    return isParentOf( wxFileName( parent ), child );
}

bool isParentOf(const wxFileName& parent, const wxString& child)
{
    return isParentOf( parent, wxFileName( child ) );
}

bool isParentOf(const wxString& parent, const wxString& child)
{
    return isParentOf( wxFileName( parent ),  wxFileName( child ) );
}

bool hasSubDirectories(wxFileName directory)
{
    ASSERT(directory.IsDir())(directory);
    wxArrayString allfiles;
    return wxDir::GetAllFiles(directory.GetLongPath(), &allfiles, wxEmptyString, wxDIR_DIRS) > 0;
}

wxFileName getExeDir()
{
    wxString exepath;
    wxFileName::SplitPath(wxStandardPaths::Get().GetExecutablePath(),&exepath,0,0);
    wxFileName dir = wxFileName(exepath,"");
    ASSERT(!dir.HasExt());
    ASSERT(!dir.HasName());
    return dir;
}

wxString getResource(wxString subdirs, wxString filename)
{
    ASSERT(!subdirs.Contains("\\"))(subdirs);
    ASSERT(subdirs.GetChar(0) != '/')(subdirs);
    ASSERT(subdirs.Last() != '/')(subdirs);

    wxFileName result(getResourcesPath());
    result.SetFullName(filename);
    if (!subdirs.IsEmpty())
    {
        wxStringTokenizer tokenizer(subdirs, "/");
        while ( tokenizer.HasMoreTokens() )
        {
            result.AppendDir(tokenizer.GetNextToken());
        }
    }
    return result.GetFullPath();
}

bool isInstalledOnWindows()
{
    return (wxStandardPaths::Get().GetExecutablePath().Contains("Program Files"));
}

bool isInstalledOnLinux()
{
    return (wxStandardPaths::Get().GetExecutablePath().StartsWith("/usr"));
}

bool isInstalled()
{
    return isInstalledOnWindows() || isInstalledOnLinux();
}

wxFileName getLogFilePath()
{
    wxFileName result(wxStandardPaths::Get().GetExecutablePath()); // Default, log in same dir as executable
    wxString nameWithProcessId; nameWithProcessId << result.GetName() << '_' << wxGetProcessId();

    if (isInstalled())
    {
        result.SetPath(wxStandardPaths::Get().GetTempDir());
        result.SetName(nameWithProcessId);
    }
    result.SetExt("log");
    // NOT: VAR_ERROR(result); -- Logging may not yet be initialized here.
    return result;
}

wxFileName getResourcesPath()
{
    wxFileName result(wxStandardPaths::Get().GetExecutablePath()); // Default, resource in subdir of dir holding executable
    result.SetFullName("");
    if (isInstalledOnLinux()) // Installed version on Linux
    {
        // GetResourcesDir() returns path with capital V (App name is Vidiot).
        result.SetPath(wxStandardPaths::Get().GetResourcesDir().Lower());
    }
    return result;
}

wxFileName getConfigFilePath()
{
    wxFileName result(wxStandardPaths::Get().GetExecutablePath()); // Using the executable's file name enables using multiple .ini files with multiple settings.
    if (isInstalled())
    {
         // Store in "C:\Users\<username>\AppData\Roaming\exename.ini" or ~/.exename.ini
        result.SetPath(wxStandardPaths::Get().GetUserConfigDir());
#ifndef _MSC_VER
        result.SetName("." + result.GetName());
#endif
    }
    result.SetExt("ini");
    return result;
}

}} // namespace
