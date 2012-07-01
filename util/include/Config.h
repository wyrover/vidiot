#ifndef CONFIG_H
#define CONFIG_H

#include <wx/config.h> // This ensures that in other parts of the code only #include "Config.h" is required
#include <wx/string.h>
#include <boost/thread/mutex.hpp>

#include "UtilLog.h"

/// This class holds everything related to the persistence of global settings.
/// Global settings include the application options but also checked menu items
/// that are preserved upon application restart.
///
/// The class also specifies getters values stored in the config file. Since default
/// values are always filled in during 'init()', the use of default values
/// is not neccesary, and therefore these methods are provided (which do not require
// defaults).

class Config
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    static void init(wxString applicationName, wxString vendorName, bool inCxxTestMode);

    static wxString getFileName();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    static bool     ReadBool  (const wxString& key);
    static long     ReadLong  (const wxString& key);
    static double   ReadDouble(const wxString& key);
    static wxString ReadString(const wxString& key);

    template <class TYPE>
    static TYPE     ReadEnum  (const wxString& key)
    {
        wxString result;
        wxString dummy;
        bool found = wxConfigBase::Get()->Read(key, &result, dummy);
        ASSERT(found)(key);
        return Enum_fromConfig(result,TYPE());
    }

    // Specific getters for dedicated attributes are only cached for performance
    static bool getShowDebugInfo();

    //////////////////////////////////////////////////////////////////////////
    // CONFIG PATHS
    //////////////////////////////////////////////////////////////////////////

    static const wxString sPathSnapClips;
    static const wxString sPathSnapCursor;
    static const wxString sPathShowBoundingBox;
    static const wxString sPathAutoLoadEnabled;
    static const wxString sPathLastOpened;
    static const wxString sPathDefaultExtension;
    static const wxString sPathLogLevel;
    static const wxString sPathLogLevelAvcodec;
    static const wxString sPathShowDebugInfoOnWidgets;
    static const wxString sPathTest;
    static const wxString sPathDefaultFrameRate;
    static const wxString sPathDefaultVideoWidth;
    static const wxString sPathDefaultVideoHeight;
    static const wxString sPathDefaultVideoScaling;
    static const wxString sPathDefaultVideoAlignment;
    static const wxString sPathMarkerBeginAddition;
    static const wxString sPathMarkerEndAddition;
    static const wxString sPathStrip;
    static const wxString sPathDefaultTransitionLength;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    static wxString sFileName;
    static bool sShowDebugInfo;
    static boost::mutex sMutex;
};

#endif // CONFIG_H