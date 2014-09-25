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

#ifndef CONFIG_H
#define CONFIG_H

#include "UtilEvent.h"
#include "UtilLog.h"
#include "UtilSingleInstance.h"

/// This class holds everything related to the persistence of global settings.
/// Global settings include the application options but also checked menu items
/// that are preserved upon application restart.
///
/// The class also specifies getters values stored in the config file. Since default
/// values are always filled in during 'init()', the use of default values
/// is not neccesary, and therefore these methods are provided (which do not require
// defaults).

DECLARE_EVENT(EVENT_CONFIG_UPDATED, EventConfigUpdated, wxString);

class Config
    : public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    , public wxFileConfig
    , public SingleInstance<Config>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Config(const wxString& appName, const wxString& vendorName, const wxString& localFilename);

    static void init(const wxString& applicationName, const wxString& vendorName, bool inCxxTestMode);

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
    static void WriteLong(const wxString& key, const long& value);
    static void WriteDouble(const wxString& key, double value);
    static void WriteString(const wxString& key, const wxString& value);

    static void OnWrite(const wxString& key);

    // Specific getters for dedicated attributes are only cached for performance
    static bool getShowDebugInfo();
    static void setShowDebugInfo(bool show); ///< For testing only

    //////////////////////////////////////////////////////////////////////////
    // WORKSPACE PERSPECTIVES
    //////////////////////////////////////////////////////////////////////////

    typedef std::map<wxString,wxString> Perspectives;

    struct WorkspacePerspectives
    {
        static Perspectives get();
        static void add(const wxString& name, const wxString& perspective);
        static void remove(const wxString& name);
        static void removeAll();
        static void save(const Perspectives& perspectives);
    };

    //////////////////////////////////////////////////////////////////////////
    // DISK ACCESS
    //////////////////////////////////////////////////////////////////////////

    static void holdWriteToDisk();
    static void releaseWriteToDisk();

    //////////////////////////////////////////////////////////////////////////
    // CONFIG PATHS
    //////////////////////////////////////////////////////////////////////////

    static const wxString sPathAutoLoadEnabled;
    static const wxString sPathBackupBeforeSaveEnabled;
    static const wxString sPathBackupBeforeSaveMaximum;
    static const wxString sPathSavePathsRelativeToProject;
    static const wxString sPathDebugIncludeScreenShot;
    static const wxString sPathDebugLogSequenceOnEdit;
    static const wxString sPathDebugMaxRenderLength;
    static const wxString sPathDebugShowCrashMenu;
    static const wxString sPathDebugShowFrameNumbers;
    static const wxString sPathDefaultAudioChannels;
    static const wxString sPathDefaultAudioSampleRate;
    static const wxString sPathDefaultExtension;
    static const wxString sPathDefaultFrameRate;
    static const wxString sPathDefaultStillImageLength;
    static const wxString sPathDefaultTransitionLength;
    static const wxString sPathDefaultVideoAlignment;
    static const wxString sPathDefaultVideoHeight;
    static const wxString sPathDefaultVideoScaling;
    static const wxString sPathDefaultVideoWidth;
    static const wxString sPathDefaultNewProjectType;
    static const wxString sPathLastOpened;
    static const wxString sPathLogLevel;
    static const wxString sPathLogLevelAvcodec;
    static const wxString sPathMakeSequenceEmptyClipLength;
    static const wxString sPathMakeSequencePrefixLength;
    static const wxString sPathMarkerBeginAddition;
    static const wxString sPathMarkerEndAddition;
    static const wxString sPathOverruleFourCC; ///< Used to overrule the FourCC for encoding MPEG4 formy Car DVD player (only swallows MPEG4 labeled with XVID)
    static const wxString sPathShowBoundingBox;
    static const wxString sPathShowDebugInfoOnWidgets;
    static const wxString sPathSnapClips;
    static const wxString sPathSnapCursor;
    static const wxString sPathStrip;
    static const wxString sPathTest;
    static const wxString sPathTestRunCurrent;
    static const wxString sPathTestRunFrom;
    static const wxString sPathTestRunOnly;
    static const wxString sPathWindowH;
    static const wxString sPathWindowMaximized;
    static const wxString sPathWindowW;
    static const wxString sPathWindowX;
    static const wxString sPathWindowY;
    static const wxString sPathWorkspacePerspectiveCurrent;
    static const wxString sPathWorkspacePerspectiveName;
    static const wxString sPathWorkspacePerspectiveSaved;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    static bool sShowDebugInfo;
    static boost::mutex sMutex;

    static bool sHold;
};

#endif
