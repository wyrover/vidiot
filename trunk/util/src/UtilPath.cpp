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

#include "UtilPath.h"

namespace util { namespace path {

wxFileName normalize( wxFileName filename )
{
    bool normalizeResult = filename.Normalize();
    ASSERT(normalizeResult)(filename);
    return filename;
}

wxString toName( wxFileName filename )
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

wxString toPath( wxFileName filename )
{
    wxString result = normalize(filename).GetFullPath();
    if ( wxEndsWithPathSeparator(result) )
    {
        result.erase(result.length() - 1);
    }
    return result;
}

bool equals( wxFileName f1, wxFileName f2 )
{
    return toPath( f1 ).IsSameAs( toPath( f2 ) );
}

bool equals( wxString f1, wxFileName f2 )
{
    return equals( wxFileName(f1), f2 );
}

bool equals( wxFileName f1, wxString f2 )
{
    return equals( f1, wxFileName( f2 ) );
}

bool equals( wxString f1, wxString f2 )
{
    return equals( wxFileName(f1), wxFileName(f2) );
}

bool isParentOf( wxFileName parent, wxFileName child )
{
    wxString sChild = toPath( child );
    wxString sParent = toPath( parent );

    return !(sChild.IsSameAs(sParent)) && sChild.StartsWith( sParent );
}

bool isParentOf( wxString parent, wxFileName child )
{
    return isParentOf( wxFileName( parent ), child );
}

bool isParentOf( wxFileName parent, wxString child )
{
    return isParentOf( parent, wxFileName( child ) );
}

bool isParentOf( wxString parent,   wxString child )
{
    return isParentOf( wxFileName( parent ),  wxFileName( child ) );
}

}} // namespace