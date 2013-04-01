#ifndef CONFIG_H
#define CONFIG_H

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

    static wxString getExeDir();

    static bool     Exists(const wxString& key);

    static bool     ReadBool  (const wxString& key);
    static long     ReadLong  (const wxString& key);
    static double   ReadDouble(const wxString& key);
    static wxString ReadString(const wxString& key);

    template <class TYPE>
    static TYPE     ReadEnum  (const wxString& key)
    {
        wxString result = ReadString(key);
        return Enum_fromConfig(result,TYPE());
    }

    static void WriteBool(const wxString& key, bool value);
    static void WriteLong(const wxString& key, long value);
    static void WriteDouble(const wxString& key, double value);
    static void WriteString(const wxString& key, wxString value);

    // Specific getters for dedicated attributes are only cached for performance
    static bool getShowDebugInfo();
    static void setShowDebugInfo(bool show); ///< For testing only

    //////////////////////////////////////////////////////////////////////////
    // DISK ACCESS
    //////////////////////////////////////////////////////////////////////////

    static void holdWriteToDisk();
    static void releaseWriteToDisk();

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
    static const wxString sPathDebugMaxRenderLength;
    static const wxString sPathDefaultFrameRate;
    static const wxString sPathDefaultVideoWidth;
    static const wxString sPathDefaultVideoHeight;
    static const wxString sPathDefaultVideoScaling;
    static const wxString sPathDefaultVideoAlignment;
    static const wxString sPathOverruleFourCC; ///< Used to overrule the FourCC for encoding MPEG4 formy Car DVD player (only swallows MPEG4 labeled with XVID)
    static const wxString sPathDefaultAudioSampleRate;
    static const wxString sPathDefaultAudioChannels;
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

    static bool sHold;
};

#endif // CONFIG_H