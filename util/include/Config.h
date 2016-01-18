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

#pragma once

#include "UtilSingleInstance.h"

#include <wx/fileconf.h> // All config handling must be done via this file, for thread safety

DECLARE_EVENT(EVENT_CONFIG_UPDATED, EventConfigUpdated, wxString);

/// This class holds everything related to the persistence of global settings.
/// Global settings include the application options but also checked menu items
/// that are preserved upon application restart.
///
/// The class also specifies getters values stored in the config file. Since default
/// values are always filled in during 'init()', the use of default values
/// is not neccesary, and therefore these methods are provided (which do not require
/// defaults).
///
/// Accessing config items via the Config::Read* methods is thread safe.
/// Config items are cached, and the access to the cache is thread safe.
class Config
    : public wxEvtHandler // MUST BE FIRST INHERITED CLASS FOR WXWIDGETS EVENTS TO BE RECEIVED.
    , public wxFileConfig
    , public SingleInstance<Config>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Config(const wxString& applicationName, const wxString& vendorName, const wxString& configFile);
    void init(bool inCxxTestMode);
    void exit();
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    static void init(const wxString& applicationName, const wxString& vendorName, bool inCxxTestMode);

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    bool exists(const wxString& key) const;

    template <typename T>
    T read(const wxString& key) const;

    template <class TYPE>
    TYPE ReadEnum(const wxString& key) const
    {
        return Enum_fromConfig(read<wxString>(key), TYPE());
    }

    template <typename T>
    void write(const wxString& key, const T& value);

    void onWrite(const wxString& key);

    // Specific getters for dedicated attributes are only cached for performance
    static bool getShowDebugInfo();
    static void setShowDebugInfo(bool show); ///< For testing only

    //////////////////////////////////////////////////////////////////////////
    // WORKSPACE PERSPECTIVES
    //////////////////////////////////////////////////////////////////////////

    typedef std::map<wxString,wxString> Perspectives;

    Perspectives getWorkspacePerspectives();
    void addWorkspacePerspective(const wxString& name, const wxString& perspective);
    void removeWorkspacePerspective(const wxString& name);
    void removeAllWorkspacePerspectives();
    void saveWorkspacePerspectives(const Perspectives& perspectives);

    //////////////////////////////////////////////////////////////////////////
    // DISK ACCESS
    //////////////////////////////////////////////////////////////////////////

    void holdWriteToDisk();
    void releaseWriteToDisk();

    //////////////////////////////////////////////////////////////////////////
    // CONFIG PATHS
    //////////////////////////////////////////////////////////////////////////

    static const wxString sPathAudioDefaultNumberOfChannels;
    static const wxString sPathAudioDefaultSampleRate;
    static const wxString sPathDebugIncludeScreenShotInDump;
    static const wxString sPathDebugLogLevel;
    static const wxString sPathDebugLogLevelAvcodec;
    static const wxString sPathDebugLogSequenceOnEdit;
    static const wxString sPathDebugMaxRenderLength;
    static const wxString sPathDebugShowCrashMenu;
    static const wxString sPathDebugShowDebugInfoOnWidgets;
    static const wxString sPathDebugShowFrameNumbers;
    static const wxString sPathFileDefaultExtension;
    static const wxString sPathMakeSequenceEmptyClipLength;
    static const wxString sPathMakeSequencePrefixLength;
    static const wxString sPathProjectAutoLoadEnabled; 
    static const wxString sPathProjectBackupBeforeSaveEnabled;
    static const wxString sPathProjectBackupBeforeSaveMaximum;
    static const wxString sPathProjectDefaultNewProjectType;
    static const wxString sPathProjectLastOpened;
    static const wxString sPathProjectSavePathsRelativeToProject;
    static const wxString sPathTestCxxMode;
    static const wxString sPathTestRunCurrent;
    static const wxString sPathTestRunFrom;
    static const wxString sPathTestRunOnly;
    static const wxString sPathTimelineAutoAddEmptyTrackWhenDragging;
    static const wxString sPathTimelineDefaultStillImageLength;
    static const wxString sPathTimelineDefaultTransitionLength;
    static const wxString sPathTimelineMarkerBeginAddition;
    static const wxString sPathTimelineMarkerEndAddition;
    static const wxString sPathTimelineSnapClips;
    static const wxString sPathTimelineSnapCursor;
    static const wxString sPathTimelineStripFromClipNames;
    static const wxString sPathTimelineLengthButtons;
    static const wxString sPathVideoDefaultAlignment;
    static const wxString sPathVideoDefaultFrameRate;
    static const wxString sPathVideoDefaultHeight;
    static const wxString sPathVideoDefaultScaling;
    static const wxString sPathVideoDefaultWidth;
    static const wxString sPathVideoOverruleFourCC; ///< Used to overrule the FourCC for encoding MPEG4 formy Car DVD player (only swallows MPEG4 labeled with XVID)
    static const wxString sPathVideoShowBoundingBox;
    static const wxString sPathWorkspaceLanguage;
    static const wxString sPathWorkspaceH;
    static const wxString sPathWorkspaceMaximized;
    static const wxString sPathWorkspacePerspectiveCurrent;
    static const wxString sPathWorkspacePerspectiveName;
    static const wxString sPathWorkspaceW;
    static const wxString sPathWorkspaceX;
    static const wxString sPathWorkspaceY;
    static const wxString sPathWorkspacePerspectiveSaved;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    std::unique_ptr<wxLocale> mLocale = nullptr;

    static bool sShowDebugInfo;

    static bool sHold;

    std::map<wxString, wxString> mCache;
    mutable boost::mutex mCacheMutex;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHDOS
    //////////////////////////////////////////////////////////////////////////

    void indexEntries();
    void updateCache();

};
