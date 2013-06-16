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