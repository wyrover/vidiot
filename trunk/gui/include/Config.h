#ifndef CONFIG_H
#define CONFIG_H

#include <wx/string.h>

namespace gui {

/// This class holds everything related to the persistence of global settings.
/// Global settings include the application options but also checked menu items
/// that are preserved upon application restart.
class Config
{
public:
// TODO MOVE FROM OPTIONS TO HERE

    //////////////////////////////////////////////////////////////////////////
    // CONFIG PATHS
    //////////////////////////////////////////////////////////////////////////

    static const wxString sPathSnapClips;
    static const wxString sPathSnapCursor;
};

} // namespace

#endif // CONFIG_H
