#ifndef CONFIG_H
#define CONFIG_H

#include <wx/string.h>
#include <wx/config.h> // This ensures that in other parts of the code only #include "Config.h" is required

namespace gui {

/// This class holds everything related to the persistence of global settings.
/// Global settings include the application options but also checked menu items
/// that are preserved upon application restart.
class Config
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    static void init(wxString applicationName, wxString vendorName);

    static wxString getFileName();

    //////////////////////////////////////////////////////////////////////////
    // CONFIG PATHS
    //////////////////////////////////////////////////////////////////////////

    static const wxString sPathSnapClips;
    static const wxString sPathSnapCursor;
    static const wxString sPathAutoLoadEnabled;
    static const wxString sPathAutoLoadFilename;
    static const wxString sPathLastOpened;
    static const wxString sPathLogLevel;
    static const wxString sPathLogFile;
    static const wxString sPathShowDebugInfoOnWidgets;
    static const wxString sPathFrameRate;
    static const wxString sPathMarkerBeginAddition;
    static const wxString sPathMarkerEndAddition;
    static const wxString sPathStrip;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    static wxString sFileName;

};

} // namespace

#endif // CONFIG_H
