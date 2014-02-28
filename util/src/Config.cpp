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

#include "Config.h"

#include <wx/confbase.h> // All config handling must be done via this file, for thread safety
#include <wx/config.h> // All config handling must be done via this file, for thread safety
#include "Enums.h"
#include "UtilFrameRate.h"
#include "UtilInitAvcodec.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"

bool Config::sShowDebugInfo(false);
boost::mutex Config::sMutex;
bool Config::sHold(false);

DEFINE_EVENT(EVENT_CONFIG_UPDATED, EventConfigUpdated, wxString);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Config::Config(const wxString& appName, const wxString& vendorName, const wxString& localFilename)
    : wxEvtHandler()
    , wxFileConfig(appName,vendorName,localFilename)
{
}

template <class T>
void setDefault(const wxString& path, const T& value)
{
    if (!wxConfigBase::Get()->Exists(path))
    {
        wxConfigBase::Get()->Write(path, value);
    }
}

void checkLong(const wxString& path, const long& lowerbound, const long& upperbound)
{
    // Check if it's actually a long
    long value(0);
    if (!wxConfigBase::Get()->Read(path, "").ToLong(&value))
    {
        wxConfigBase::Get()->DeleteEntry(path);
    }
    else
    {
        bool found = wxConfigBase::Get()->Read(path, &value, 0);
        ASSERT(found);
        if (value < lowerbound || value > upperbound)
        {
            wxConfigBase::Get()->DeleteEntry(path);
        }
    }
}

void checkBool(const wxString& path)
{
    checkLong(path,0,1);
}

#define checkEnum(path, ENUMNAME) \
{\
    wxString s = wxConfigBase::Get()->Read(path, ""); \
    ENUMNAME dummy = ENUMNAME ## _MAX; \
    ENUMNAME e = Enum_fromConfig(s, dummy); \
    if (e == ENUMNAME ## _MAX) { wxConfigBase::Get()->DeleteEntry(path); } \
}

void checkEnumFromMap(const wxString& path, const boost::bimap<int, wxString>& bimap)
{
    wxString s = wxConfigBase::Get()->Read(path, "");
    if (bimap.right.find(s) == bimap.right.end())
    {
        wxConfigBase::Get()->DeleteEntry(path);
    }
}

// static
void Config::init(const wxString& applicationName, const wxString& vendorName, bool inCxxTestMode)
{
    // Initialize config object. Will be destructed by wxWidgets at the end of the application
    wxString ConfigFile(getFileName());
    VAR_ERROR(ConfigFile);
    wxConfigBase::Set(new Config(applicationName, vendorName, ConfigFile));
    wxConfigBase::Get()->Write(Config::sPathTest, inCxxTestMode);

    // Check values, delete from config if incorrect
    checkLong(Config::sPathMakeSequenceEmptyClipLength, 0, 100000);
    checkLong(Config::sPathMakeSequencePrefixLength, 1, 100);
    checkBool(Config::sPathAutoLoadEnabled);
    checkBool(Config::sPathBackupBeforeSaveEnabled);
    checkLong(Config::sPathBackupBeforeSaveMaximum, 0, 10000);
    checkLong(Config::sPathDefaultStillImageLength, 1, 10000);
    checkLong(Config::sPathDefaultTransitionLength, 4, 10000);
    checkLong(Config::sPathDefaultVideoWidth, 10, 10000);
    checkLong(Config::sPathDefaultVideoHeight, 10, 10000);
    checkEnum(Config::sPathDefaultVideoScaling, model::VideoScaling);
    checkEnum(Config::sPathDefaultVideoAlignment, model::VideoAlignment);
    checkLong(Config::sPathDefaultAudioSampleRate, 100, 100000);
    checkLong(Config::sPathDefaultAudioChannels, 1, 2);
    checkEnum(Config::sPathLogLevel, LogLevel);
    checkEnumFromMap(Config::sPathLogLevelAvcodec, Avcodec::mapAvcodecLevels);
    checkLong(Config::sPathMarkerBeginAddition, 0, 10000);
    checkLong(Config::sPathMarkerEndAddition, 0, 10000);
    checkBool(Config::sPathShowDebugInfoOnWidgets);
    checkBool(Config::sPathSnapClips);
    checkBool(Config::sPathSnapCursor);
    checkBool(Config::sPathShowBoundingBox);
    checkLong(Config::sPathDebugMaxRenderLength, 0, 1000000);
    checkBool(Config::sPathDebugShowCrashMenu);
    checkBool(Config::sPathDebugIncludeScreenShot);
    checkBool(Config::sPathDebugLogSequenceOnEdit);

    // Set all defaults here
    setDefault(Config::sPathAutoLoadEnabled, false);
    setDefault(Config::sPathBackupBeforeSaveEnabled, true);
    setDefault(Config::sPathBackupBeforeSaveMaximum, 10);
    setDefault(Config::sPathDebugIncludeScreenShot, true);
    setDefault(Config::sPathDebugLogSequenceOnEdit, false);
    setDefault(Config::sPathDebugMaxRenderLength, 0); // Per default, render all
    setDefault(Config::sPathDebugShowCrashMenu, false);
    setDefault(Config::sPathDefaultAudioChannels, 2);
    setDefault(Config::sPathDefaultAudioSampleRate, 44100);
    setDefault(Config::sPathDefaultExtension, "avi");
    setDefault(Config::sPathDefaultStillImageLength, 150);
    setDefault(Config::sPathDefaultTransitionLength, 24);
    setDefault(Config::sPathDefaultVideoAlignment, model::VideoAlignment_toString(model::VideoAlignmentCenter).c_str());
    setDefault(Config::sPathDefaultVideoHeight, 720);
    setDefault(Config::sPathDefaultVideoScaling, model::VideoScaling_toString(model::VideoScalingFitToFill).c_str());
    setDefault(Config::sPathDefaultVideoWidth, 1280);
    setDefault(Config::sPathLastOpened, "");
    setDefault(Config::sPathLogLevel, LogLevel_toString(LogWarning).c_str());
    setDefault(Config::sPathLogLevelAvcodec, Avcodec::getDefaultLogLevel());
    setDefault(Config::sPathMakeSequenceEmptyClipLength, 0);
    setDefault(Config::sPathMakeSequencePrefixLength, 14);
    setDefault(Config::sPathMarkerBeginAddition, 0);
    setDefault(Config::sPathMarkerEndAddition, 0);
    setDefault(Config::sPathShowBoundingBox, true);
    setDefault(Config::sPathShowDebugInfoOnWidgets, false);
    setDefault(Config::sPathSnapClips, true);
    setDefault(Config::sPathSnapCursor, true);
    setDefault(Config::sPathStrip, "scene'2010");
    setDefault(Config::sPathTestRunCurrent, "");
    setDefault(Config::sPathTestRunFrom, "");
    setDefault(Config::sPathTestRunOnly, "");
    setDefault(Config::sPathWindowH, -1);
    setDefault(Config::sPathWindowMaximized, false);
    setDefault(Config::sPathWindowW, -1);
    setDefault(Config::sPathWindowX, -1);
    setDefault(Config::sPathWindowY, -1);
    setDefault(Config::sPathWorkspacePerspectiveCurrent,"");

    // Special cases checking and default handling
    wxString frameRate = wxConfigBase::Get()->Read(Config::sPathDefaultFrameRate, "");
    FrameRate fr(frameRate);
    if (!fr.toString().IsSameAs(frameRate))
    {
        WriteString(Config::sPathDefaultFrameRate, fr.toString());
    }

    wxConfigBase::Get()->Flush();

    // Read cached values here
    Log::setReportingLevel(LogLevelConverter::readConfigValue(Config::sPathLogLevel));
    sShowDebugInfo = Config::ReadBool(Config::sPathShowDebugInfoOnWidgets);

    Avcodec::configureLog();
}

// static
wxString Config::getFileName()
{
    wxFileName configFile(wxStandardPaths::Get().GetExecutablePath()); // Using the executable's file name enables using multiple .ini files with multiple settings.
    configFile.SetExt("ini");

    if (configFile.GetFullPath().Contains("Program Files"))
    {
        // When running from "Program Files" (installed version), store this file elsewhere to avoid being unable to write.
        configFile.SetPath(wxStandardPaths::Get().GetUserConfigDir()); // Store in "C:\Users\<username>\AppData\Roaming\<executablename>.ini"
    }

    return configFile.GetFullPath();
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

// static
wxString Config::getExeDir()
{
    wxString exepath;
    wxFileName::SplitPath(wxStandardPaths::Get().GetExecutablePath(),&exepath,0,0);
    wxFileName dir = wxFileName(exepath,"");
    ASSERT(!dir.HasExt());
    ASSERT(!dir.HasName());
    wxString configdir = wxFileName::GetCwd();
    return dir.GetFullPath();
}

template <class T>
T readWithoutDefault(const wxString& path)
{
    T result = T();
    T dummy = T();
    bool found = wxConfigBase::Get()->Read(path, &result, dummy);
    ASSERT(found)(path);
    return result;
}

//static
bool Config::Exists(const wxString& key)
{
    return wxConfigBase::Get()->Exists(key);
}

// static
bool Config::ReadBool(const wxString& key)
{
    boost::mutex::scoped_lock lock(sMutex);
    return readWithoutDefault<bool>(key);
}

// static
long Config::ReadLong(const wxString& key)
{
    boost::mutex::scoped_lock lock(sMutex);
    return readWithoutDefault<long>(key);
}

// static
double Config::ReadDouble(const wxString& key)
{
    boost::mutex::scoped_lock lock(sMutex);
    return readWithoutDefault<double>(key);
}

// static
wxString Config::ReadString(const wxString& key)
{
    boost::mutex::scoped_lock lock(sMutex);
    return readWithoutDefault<wxString>(key);
}

// static
void Config::WriteBool(const wxString& key, bool value)
{
    boost::mutex::scoped_lock lock(sMutex);
    bool result = wxConfigBase::Get()->Write(key, value);
    ASSERT(result);
    OnWrite(key);
}

// static
void Config::WriteLong(const wxString& key, const long& value)
{
    boost::mutex::scoped_lock lock(sMutex);
    bool result = wxConfigBase::Get()->Write(key, value);
    ASSERT(result);
    OnWrite(key);
}

// static
void Config::WriteDouble(const wxString& key, double value)
{
    boost::mutex::scoped_lock lock(sMutex);
    bool result = wxConfigBase::Get()->Write(key, value);
    ASSERT(result);
    OnWrite(key);
}

// static
void Config::WriteString(const wxString& key, const wxString& value)
{
    boost::mutex::scoped_lock lock(sMutex);
    bool result = wxConfigBase::Get()->Write(key, value);
    ASSERT(result);
    OnWrite(key);
}

// static
void Config::OnWrite(const wxString& key)
{
    Config* cfg = dynamic_cast<Config*>(wxConfigBase::Get());
    ASSERT_NONZERO(cfg);
    cfg->QueueEvent(new EventConfigUpdated(key));
    if (!sHold)
    {
        bool result = wxConfigBase::Get()->Flush();
        ASSERT(result);
    }
}

// static
bool Config::getShowDebugInfo()
{
    return sShowDebugInfo;
}

// static
void Config::setShowDebugInfo(bool show)
{
    sShowDebugInfo = show;
}

//////////////////////////////////////////////////////////////////////////
// WORKSPACE PERSPECTIVES
//////////////////////////////////////////////////////////////////////////

Config::Perspectives Config::WorkspacePerspectives::get()
{
    Config::Perspectives result;
    for (int i = 1; i < 100; ++i) // More than 100 not supported
    {
        wxString pathName = sPathWorkspacePerspectiveName + wxString::Format("%d",i);
        wxString pathSaved = sPathWorkspacePerspectiveSaved + wxString::Format("%d",i);
        if (!Exists(pathName) || !Exists(pathSaved))
        {
            break;
        }
        result[ReadString(pathName)] = ReadString(pathSaved); // Also avoids duplicates
    }
    return result;
}

void Config::WorkspacePerspectives::add(const wxString& name, const wxString& perspective)
{
    Config::Perspectives perspectives = get();
    perspectives[name] = perspective;
    save(perspectives);
}

void Config::WorkspacePerspectives::remove(const wxString& name)
{
    Config::Perspectives perspectives = get();
    perspectives.erase(name);
    save(perspectives);
}

void Config::WorkspacePerspectives::removeAll()
{
    for (int i = 1; i < 100; ++i) // More than 100 not supported
    {
        wxConfigBase::Get()->DeleteEntry(sPathWorkspacePerspectiveName + wxString::Format("%d",i));
        wxConfigBase::Get()->DeleteEntry(sPathWorkspacePerspectiveSaved + wxString::Format("%d",i));
    }
    wxConfigBase::Get()->Flush();
}

void Config::WorkspacePerspectives::save(const Perspectives& perspectives)
{
    removeAll();
    int i = 1;
    for ( Config::Perspectives::value_type name_perspective : perspectives )
    {
        wxString pathName = sPathWorkspacePerspectiveName + wxString::Format("%d",i);
        wxString pathSaved = sPathWorkspacePerspectiveSaved + wxString::Format("%d",i);
        WriteString(pathName, name_perspective.first);
        WriteString(pathSaved, name_perspective.second);
        ++i;
    }
    wxConfigBase::Get()->Flush();
}

//////////////////////////////////////////////////////////////////////////
// DISK ACCESS
//////////////////////////////////////////////////////////////////////////

// static
void Config::holdWriteToDisk()
{
    boost::mutex::scoped_lock lock(sMutex);
    ASSERT(!sHold);
    sHold = true;
}

// static
void Config::releaseWriteToDisk()
{
    boost::mutex::scoped_lock lock(sMutex);
    ASSERT(sHold);
    sHold = false;
    bool result = wxConfigBase::Get()->Flush();
    ASSERT(result);
}

//////////////////////////////////////////////////////////////////////////
// CONFIG PATHS
//////////////////////////////////////////////////////////////////////////

const wxString Config::sPathAutoLoadEnabled             ("/Project/AutoLoadEnabled");
const wxString Config::sPathBackupBeforeSaveEnabled     ("/Project/BackupBeforeSaveEnabled");
const wxString Config::sPathBackupBeforeSaveMaximum     ("/Project/BackupBeforeSaveMaximumFileCount");
const wxString Config::sPathDebugIncludeScreenShot      ("/Debug/Screenshot");
const wxString Config::sPathDebugLogSequenceOnEdit      ("/Debug/LogSequenceOnEdit");
const wxString Config::sPathDebugMaxRenderLength        ("/Debug/MaxRenderLength");
const wxString Config::sPathDebugShowCrashMenu          ("/Debug/ShowCrashMenu");
const wxString Config::sPathDefaultAudioChannels        ("/Audio/DefaultNumberOfChannels");
const wxString Config::sPathDefaultAudioSampleRate      ("/Audio/DefaultSampleRate");
const wxString Config::sPathDefaultExtension            ("/File/DefaultExtension");
const wxString Config::sPathDefaultFrameRate            ("/Video/DefaultFrameRate");
const wxString Config::sPathDefaultStillImageLength     ("/Timeline/DefaultStillImageLength");
const wxString Config::sPathDefaultTransitionLength     ("/Timeline/DefaultTransitionLength");
const wxString Config::sPathDefaultVideoAlignment       ("/Video/DefaultVideoAlignment");
const wxString Config::sPathDefaultVideoHeight          ("/Video/DefaultHeight");
const wxString Config::sPathDefaultVideoScaling         ("/Video/DefaultVideoScaling");
const wxString Config::sPathDefaultVideoWidth           ("/Video/DefaultWidth");
const wxString Config::sPathLastOpened                  ("/Project/LastOpened");
const wxString Config::sPathLogLevel                    ("/Debug/LogLevel");
const wxString Config::sPathLogLevelAvcodec             ("/Debug/LogLevelAvcodec");
const wxString Config::sPathMakeSequenceEmptyClipLength ("/MakeSequence/EmptyLength");
const wxString Config::sPathMakeSequencePrefixLength    ("/MakeSequence/PrefixLength");
const wxString Config::sPathMarkerBeginAddition         ("/Timeline/MarkerBeginAddition");
const wxString Config::sPathMarkerEndAddition           ("/Timeline/MarkerEndAddition");
const wxString Config::sPathOverruleFourCC              ("/Video/FourCC");
const wxString Config::sPathShowBoundingBox             ("/View/BoundingBox");
const wxString Config::sPathShowDebugInfoOnWidgets      ("/Debug/Show");
const wxString Config::sPathSnapClips                   ("/View/SnapClips");
const wxString Config::sPathSnapCursor                  ("/View/SnapCursor");
const wxString Config::sPathStrip                       ("/Timeline/Strip");
const wxString Config::sPathTest                        ("/Test/CxxTestMode");
const wxString Config::sPathTestRunOnly                 ("/Test/RunOnly");
const wxString Config::sPathTestRunFrom                 ("/Test/RunFrom");
const wxString Config::sPathTestRunCurrent              ("/Test/RunCurrent");
const wxString Config::sPathWindowMaximized             ("/Workspace/Maximized");
const wxString Config::sPathWindowW                     ("/Workspace/W");
const wxString Config::sPathWindowH                     ("/Workspace/H");
const wxString Config::sPathWindowX                     ("/Workspace/X");
const wxString Config::sPathWindowY                     ("/Workspace/Y");
const wxString Config::sPathWorkspacePerspectiveName    ("/Workspace/PerspectiveName");
const wxString Config::sPathWorkspacePerspectiveSaved   ("/Workspace/PerspectiveSaved");
const wxString Config::sPathWorkspacePerspectiveCurrent ("/Workspace/PerspectiveCurrent");