#include "UtilPath.h"

namespace util { namespace path {

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
            result = filename.GetVolume();
        }
    }
    else
    {
        result = filename.GetFullName();
    }
    return result;
}

wxFileName normalize( wxFileName filename )
{
    // GetLongPath needed to remove ~x for short windows paths
    return wxFileName(filename.GetLongPath(),"");
}

}} // namespace
