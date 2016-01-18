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

#include "Config.h"

#include "Enums.h"
#include "UtilFrameRate.h"
#include "UtilInitAvcodec.h"
#include "UtilLocale.h"
#include "UtilPath.h"

// static
bool Config::sShowDebugInfo(false);
bool Config::sHold(false);
std::unique_ptr<wxLocale> Config::sLocale = nullptr;

DEFINE_EVENT(EVENT_CONFIG_UPDATED, EventConfigUpdated, wxString);

// todo make thread-safe for reading (cache all values), ensure writing done in main thread, and re-read upon change

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

void checkEnumFromMap(const wxString& path, const std::map<int, wxString>& map)
{
    wxString s = wxConfigBase::Get()->Read(path, "");
    for (auto kvp : map) { if (kvp.second == s) { return;} }
    wxConfigBase::Get()->DeleteEntry(path);
}

void checkColour(const wxString& path)
{
    wxRegEx reColourDecimal { wxT("^([[:digit:]][[:digit:]]?[[:digit:]]?),([[:digit:]][[:digit:]]?[[:digit:]]?),([[:digit:]][[:digit:]]?[[:digit:]]?)$") };
    ASSERT(reColourDecimal.IsValid());
    wxString s = wxConfigBase::Get()->Read(path, "");
    if (reColourDecimal.Matches(s) &&
        (wxAtoi(reColourDecimal.GetMatch(s, 1)) >= 0) &&
        (wxAtoi(reColourDecimal.GetMatch(s, 1)) < 256) &&
        (wxAtoi(reColourDecimal.GetMatch(s, 2)) >= 0) &&
        (wxAtoi(reColourDecimal.GetMatch(s, 2)) < 256) &&
        (wxAtoi(reColourDecimal.GetMatch(s, 3)) >= 0) &&
        (wxAtoi(reColourDecimal.GetMatch(s, 3)) < 256))
    {
        return;
    }
    wxConfigBase::Get()->DeleteEntry(path);
}

// static
void Config::init(const wxString& applicationName, const wxString& vendorName, bool inCxxTestMode)
{
    // Initialize config object. Will be destructed by wxWidgets at the end of the application
    wxString ConfigFile(util::path::getConfigFilePath().GetFullPath());
    VAR_ERROR(ConfigFile);
    wxConfigBase::Set(new Config(applicationName, vendorName, ConfigFile));
    wxConfigBase::Get()->Write(Config::sPathTestCxxMode, inCxxTestMode);

    // Initialize language before anything else to ensure that any strings initialized in the 'enum checks' lookups
    // are properly translated. Example: Avcodec log level mapping.
    setDefault(Config::sPathWorkspaceLanguage, getDefaultLanguage());
    wxString Language{ getLanguageCode() };
    std::vector<std::pair<wxString, wxString>> supported{ getSupportedLanguages() };
    if (std::find_if(supported.begin(), supported.end(), [Language](std::pair<wxString, wxString> name_and_code) { return name_and_code.second == Language; }) == supported.end())
    {
        // Unsupported language code in config
        Language = getDefaultLanguage();
        if (std::find_if(supported.begin(), supported.end(), [Language](std::pair<wxString, wxString> name_and_code) { return name_and_code.second == Language; }) == supported.end())
        {
            // Unsupported default language
            Language = "en";
        }
    }

    wxLocale::AddCatalogLookupPathPrefix(util::path::getLanguagesPath().GetFullPath());
    sLocale.reset(new wxLocale());
    wxLanguage languageId{ getLanguageId(Language) };
    bool LocaleInitialization = sLocale->Init(languageId, wxLOCALE_LOAD_DEFAULT);
    bool WxTranslations = sLocale->AddCatalog("vidiotwx", languageId);
    bool VidiotTranslations = sLocale->AddCatalog("vidiot", languageId); // Load last: This gives vidiot strings higher priority than wx strings (exmple the Copy string in 'nl_NL')
    VAR_ERROR(Language)(LocaleInitialization)(VidiotTranslations)(WxTranslations);
    WriteString(Config::sPathWorkspaceLanguage, Language);

    // Check values, delete from config if incorrect
    checkLong(Config::sPathMakeSequenceEmptyClipLength, 0, 100000);
    checkLong(Config::sPathMakeSequencePrefixLength, 1, 100);
    checkBool(Config::sPathProjectAutoLoadEnabled);
    checkBool(Config::sPathProjectBackupBeforeSaveEnabled);
    checkLong(Config::sPathProjectBackupBeforeSaveMaximum, 0, 10000);
    checkBool(Config::sPathProjectSavePathsRelativeToProject);
    checkBool(Config::sPathTimelineAutoAddEmptyTrackWhenDragging);
    checkLong(Config::sPathTimelineDefaultStillImageLength, 1, 10000);
    checkLong(Config::sPathTimelineDefaultTransitionLength, 4, 10000);
    checkLong(Config::sPathVideoDefaultWidth, 10, 10000);
    checkLong(Config::sPathVideoDefaultHeight, 10, 10000);
    checkEnum(Config::sPathVideoDefaultScaling, model::VideoScaling);
    checkEnum(Config::sPathVideoDefaultAlignment, model::VideoAlignment);
    checkLong(Config::sPathAudioDefaultSampleRate, 100, 100000);
    checkLong(Config::sPathAudioDefaultNumberOfChannels, 1, 2);
    checkEnum(Config::sPathDebugLogLevel, LogLevel);
    checkEnum(Config::sPathProjectDefaultNewProjectType, model::DefaultNewProjectWizardStart);
    checkEnum(Config::sPathDebugLogLevelAvcodec, LogLevelAvcodec);
    checkLong(Config::sPathTimelineMarkerBeginAddition, 0, 10000);
    checkLong(Config::sPathTimelineMarkerEndAddition, 0, 10000);
    checkBool(Config::sPathDebugShowDebugInfoOnWidgets);
    checkBool(Config::sPathTimelineSnapClips);
    checkBool(Config::sPathTimelineSnapCursor);
    checkBool(Config::sPathVideoShowBoundingBox);
    checkLong(Config::sPathDebugMaxRenderLength, 0, 1000000);
    checkBool(Config::sPathDebugShowCrashMenu);
    checkBool(Config::sPathDebugShowFrameNumbers);
    checkBool(Config::sPathDebugIncludeScreenShotInDump);
    checkBool(Config::sPathDebugLogSequenceOnEdit);

    // Set all defaults here
    setDefault(Config::sPathProjectAutoLoadEnabled, !inCxxTestMode); // Only in non-test mode auto load is allowed.
    setDefault(Config::sPathProjectBackupBeforeSaveEnabled, true);
    setDefault(Config::sPathProjectBackupBeforeSaveMaximum, 10);
    setDefault(Config::sPathProjectSavePathsRelativeToProject, true);
    setDefault(Config::sPathDebugIncludeScreenShotInDump, true);
    setDefault(Config::sPathDebugLogSequenceOnEdit, false);
    setDefault(Config::sPathDebugMaxRenderLength, 0); // Per default, render all
    setDefault(Config::sPathDebugShowCrashMenu, false);
    setDefault(Config::sPathDebugShowFrameNumbers, false);
    setDefault(Config::sPathAudioDefaultNumberOfChannels, 2);
    setDefault(Config::sPathAudioDefaultSampleRate, 44100);
    setDefault(Config::sPathFileDefaultExtension, "avi");
    setDefault(Config::sPathTimelineAutoAddEmptyTrackWhenDragging, true);
    setDefault(Config::sPathTimelineDefaultStillImageLength, 150);
    setDefault(Config::sPathTimelineDefaultTransitionLength, 20); // Divisible by 4 for automated tests
    setDefault(Config::sPathVideoDefaultAlignment, model::VideoAlignment_toString(model::VideoAlignmentCenter));
    setDefault(Config::sPathVideoDefaultHeight, 720);
    setDefault(Config::sPathVideoDefaultScaling, model::VideoScaling_toString(model::VideoScalingFitToFill));
    setDefault(Config::sPathVideoDefaultWidth, 1280);
    setDefault(Config::sPathProjectLastOpened, "");
    setDefault(Config::sPathDebugLogLevel, LogLevel_toString(LogInfo));
    setDefault(Config::sPathProjectDefaultNewProjectType, model::DefaultNewProjectWizardStart_toString(model::DefaultNewProjectWizardStartFolder));
    setDefault(Config::sPathDebugLogLevelAvcodec, LogLevelAvcodec_toString(LogLevelAvcodecError));
    setDefault(Config::sPathMakeSequenceEmptyClipLength, 0);
    setDefault(Config::sPathMakeSequencePrefixLength, 14);
    setDefault(Config::sPathTimelineMarkerBeginAddition, 0);
    setDefault(Config::sPathTimelineMarkerEndAddition, 0);
    setDefault(Config::sPathTimelineLengthButtons, "250,500,1000,1500,2000,2500,3000,5000,10000"); // Keep in sync with defaults in DetailsClip
    setDefault(Config::sPathVideoShowBoundingBox, true);
    setDefault(Config::sPathDebugShowDebugInfoOnWidgets, false);
    setDefault(Config::sPathTimelineSnapClips, true);
    setDefault(Config::sPathTimelineSnapCursor, true);
    setDefault(Config::sPathTimelineStripFromClipNames, "scene'2012");
    setDefault(Config::sPathTestRunCurrent, "");
    setDefault(Config::sPathTestRunFrom, "");
    setDefault(Config::sPathTestRunOnly, "");
    setDefault(Config::sPathWorkspaceH, -1);
    setDefault(Config::sPathWorkspaceMaximized, false);
    setDefault(Config::sPathWorkspaceW, -1);
    setDefault(Config::sPathWorkspaceX, -1);
    setDefault(Config::sPathWorkspaceY, -1);
    setDefault(Config::sPathWorkspacePerspectiveCurrent,"");

    if (inCxxTestMode)
    {
        WriteString(Config::sPathProjectDefaultNewProjectType, model::DefaultNewProjectWizardStart_toString(model::DefaultNewProjectWizardStartNone));
    }

    // Special cases checking and default handling
    wxString frameRate = wxConfigBase::Get()->Read(Config::sPathVideoDefaultFrameRate, "");
    FrameRate fr(frameRate);
    if (!fr.toString().IsSameAs(frameRate))
    {
        WriteString(Config::sPathVideoDefaultFrameRate, fr.toString());
    }

    Config::get().updateCache();
    wxConfigBase::Get()->Flush();

    // Read cached values here
    Log::setReportingLevel(LogLevelConverter::readConfigValue(Config::sPathDebugLogLevel));
    sShowDebugInfo = Config::get().ReadBool(Config::sPathDebugShowDebugInfoOnWidgets);

    Avcodec::configureLog();
}

// static
void Config::exit()
{
    sLocale.reset();
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

bool Config::Exists(const wxString& key) const
{
    boost::mutex::scoped_lock lock(mCacheMutex);
    return mCache.find(key) != mCache.end();
}

template <> bool Config::read(const wxString& key) const
{
    return ReadBool(key);
}

template <> long Config::read(const wxString& key) const
{
    return ReadLong(key);
}

template <> double Config::read(const wxString& key) const
{
    return ReadDouble(key);
}

template <> wxString Config::read(const wxString& key) const
{
    return ReadString(key);
}

template <> wxColour Config::read(const wxString& key) const
{
    return ReadColour(key);
}

bool Config::ReadBool(const wxString& key) const // todo obsolete use templated methods
{
    return ReadLong(key) == 1;
}

long Config::ReadLong(const wxString& key) const
{
    wxString value{ ReadString(key) };
    long result{ 0 };
    bool ok{ value.ToLong(&result) };
    ASSERT(ok)(value)(key);
    return result;
}

double Config::ReadDouble(const wxString& key) const
{
    wxString value{ ReadString(key) };
    double result{ 0.0 };
    bool ok{ value.ToDouble(&result) };
    ASSERT(ok)(value)(key);
    return result;
}

wxString Config::ReadString(const wxString& key) const
{
    wxString value;
    {
        boost::mutex::scoped_lock lock(mCacheMutex);
        ASSERT_MAP_CONTAINS(mCache, key);
        value = mCache.find(key)->second;
    }
    return value;
}

wxColour Config::ReadColour(const wxString& key) const
{
    wxString s{ ReadString(key) };  
    wxRegEx reColourDecimal{ "^([[:digit:]][[:digit:]]?[[:digit:]]?),([[:digit:]][[:digit:]]?[[:digit:]]?),([[:digit:]][[:digit:]]?[[:digit:]]?)$" };
    ASSERT(reColourDecimal.IsValid());
    ASSERT(reColourDecimal.Matches(s))(key)(s);
    int r{ wxAtoi(reColourDecimal.GetMatch(s, 1)) };
    int g{ wxAtoi(reColourDecimal.GetMatch(s, 2)) };
    int b{ wxAtoi(reColourDecimal.GetMatch(s, 3)) };
    ASSERT_MORE_THAN_EQUALS(r,0);
    ASSERT_LESS_THAN_EQUALS(r,255);
    ASSERT_MORE_THAN_EQUALS(g,0);
    ASSERT_LESS_THAN_EQUALS(g,255);
    ASSERT_MORE_THAN_EQUALS(b,0);
    ASSERT_LESS_THAN_EQUALS(b,255);
    return wxColour{ static_cast<unsigned char>(r), static_cast<unsigned char>(g), static_cast<unsigned char>(b) };
}

template <> void Config::write(const wxString& key, const bool& value)
{
    WriteBool(key, value);
}

template <> void Config::write(const wxString& key, const long& value)
{
    WriteLong(key, value);
}

template <> void Config::write(const wxString& key, const double& value)
{
    WriteDouble(key, value);
}

template <> void Config::write(const wxString& key, const wxString& value)
{
    WriteString(key, value);
}

// static
void Config::WriteBool(const wxString& key, bool value)
{
    ASSERT(wxThread::IsMain());
    bool result = wxConfigBase::Get()->Write(key, value);
    ASSERT(result);
    OnWrite(key);
}

// static
void Config::WriteLong(const wxString& key, const long& value)
{
    ASSERT(wxThread::IsMain());
    bool result = wxConfigBase::Get()->Write(key, value);
    ASSERT(result);
    OnWrite(key);
}

// static
void Config::WriteDouble(const wxString& key, double value)
{
    ASSERT(wxThread::IsMain());
    bool result = wxConfigBase::Get()->Write(key, value);
    ASSERT(result);
    OnWrite(key);
}

// static
void Config::WriteString(const wxString& key, const wxString& value)
{
    ASSERT(wxThread::IsMain());
    bool result = wxConfigBase::Get()->Write(key, value);
    ASSERT(result);
    OnWrite(key);
}

// static
void Config::OnWrite(const wxString& key)
{
    ASSERT(wxThread::IsMain());
    Config* cfg = dynamic_cast<Config*>(wxConfigBase::Get());
    ASSERT_NONZERO(cfg);
    cfg->QueueEvent(new EventConfigUpdated(key));
    if (!sHold)
    {
        Config::get().updateCache();

        // Do not check if writing succeeded:
        // Typical case in which writing the config file may fail:
        // Open a lot of Vidiot windows, then close all those windows
        // simultaneously (on Windows, right click on icon and select
        // 'Close all windows'). Then, all those applications trying
        // to write the config file conflict. Could try to write again.
        // However, what version then has the proper - latest - changes???
        wxConfigBase::Get()->Flush();
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
    ASSERT(wxThread::IsMain());
    Config::Perspectives result;
    for (int i = 1; i < 100; ++i) // More than 100 not supported
    {
        wxString pathName = sPathWorkspacePerspectiveName + wxString::Format("%d",i);
        wxString pathSaved = sPathWorkspacePerspectiveSaved + wxString::Format("%d",i);
        if (!Config::get().Exists(pathName) || !Config::get().Exists(pathSaved))
        {
            break;
        }
        result[Config::get().ReadString(pathName)] = Config::get().ReadString(pathSaved); // Also avoids duplicates
    }
    return result;
}

void Config::WorkspacePerspectives::add(const wxString& name, const wxString& perspective)
{
    ASSERT(wxThread::IsMain());
    Config::Perspectives perspectives = get();
    perspectives[name] = perspective;
    save(perspectives);
}

void Config::WorkspacePerspectives::remove(const wxString& name)
{
    ASSERT(wxThread::IsMain());
    Config::Perspectives perspectives = get();
    perspectives.erase(name);
    save(perspectives);
}

void Config::WorkspacePerspectives::removeAll()
{
    ASSERT(wxThread::IsMain());
    for (int i = 1; i < 100; ++i) // More than 100 not supported
    {
        wxConfigBase::Get()->DeleteEntry(sPathWorkspacePerspectiveName + wxString::Format("%d",i));
        wxConfigBase::Get()->DeleteEntry(sPathWorkspacePerspectiveSaved + wxString::Format("%d",i));
    }
    Config::get().updateCache();
    wxConfigBase::Get()->Flush();
}

void Config::WorkspacePerspectives::save(const Perspectives& perspectives)
{
    ASSERT(wxThread::IsMain());
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
    Config::get().updateCache();
    wxConfigBase::Get()->Flush();
}

//////////////////////////////////////////////////////////////////////////
// DISK ACCESS
//////////////////////////////////////////////////////////////////////////

// static
void Config::holdWriteToDisk()
{
    ASSERT(wxThread::IsMain());
    ASSERT(!sHold);
    sHold = true;
}

// static
void Config::releaseWriteToDisk()
{
    ASSERT(wxThread::IsMain());
    ASSERT(sHold);
    sHold = false;
    updateCache();
    bool result = wxConfigBase::Get()->Flush();
    ASSERT(result);
}

//////////////////////////////////////////////////////////////////////////
// CONFIG PATHS
//////////////////////////////////////////////////////////////////////////

const wxString Config::sPathAudioDefaultNumberOfChannels("/Audio/DefaultNumberOfChannels");
const wxString Config::sPathAudioDefaultSampleRate("/Audio/DefaultSampleRate");
const wxString Config::sPathDebugIncludeScreenShotInDump("/Debug/IncludeScreenshotInDump");
const wxString Config::sPathDebugLogLevel("/Debug/LogLevel");
const wxString Config::sPathDebugLogLevelAvcodec("/Debug/LogLevelAvcodec");
const wxString Config::sPathDebugLogSequenceOnEdit("/Debug/LogSequenceOnEdit");
const wxString Config::sPathDebugMaxRenderLength("/Debug/MaxRenderLength");
const wxString Config::sPathDebugShowCrashMenu("/Debug/ShowCrashMenu");
const wxString Config::sPathDebugShowDebugInfoOnWidgets("/Debug/ShowDebugInfoOnWidgets");
const wxString Config::sPathDebugShowFrameNumbers("/Debug/ShowFrameNumbers");
const wxString Config::sPathFileDefaultExtension("/File/DefaultExtension");
const wxString Config::sPathMakeSequenceEmptyClipLength("/MakeSequence/EmptyClipLength");
const wxString Config::sPathMakeSequencePrefixLength("/MakeSequence/PrefixLength");
const wxString Config::sPathProjectAutoLoadEnabled("/Project/AutoLoadEnabled");
const wxString Config::sPathProjectBackupBeforeSaveEnabled("/Project/BackupBeforeSaveEnabled");
const wxString Config::sPathProjectBackupBeforeSaveMaximum("/Project/BackupBeforeSaveMaximumFileCount");
const wxString Config::sPathProjectDefaultNewProjectType("/Project/DefaultNewProjectType");
const wxString Config::sPathProjectLastOpened("/Project/LastOpened");
const wxString Config::sPathProjectSavePathsRelativeToProject("/Project/SavePathsRelativeToProject");
const wxString Config::sPathTestCxxMode("/Test/CxxTestMode");
const wxString Config::sPathTestRunCurrent("/Test/RunCurrent");
const wxString Config::sPathTestRunFrom("/Test/RunFrom");
const wxString Config::sPathTestRunOnly("/Test/RunOnly");
const wxString Config::sPathTimelineAutoAddEmptyTrackWhenDragging("/Timeline/AutoAddEmptyTrackWhenDragging");
const wxString Config::sPathTimelineDefaultStillImageLength("/Timeline/DefaultStillImageLength");
const wxString Config::sPathTimelineDefaultTransitionLength("/Timeline/DefaultTransitionLength");
const wxString Config::sPathTimelineMarkerBeginAddition("/Timeline/MarkerBeginAddition");
const wxString Config::sPathTimelineMarkerEndAddition("/Timeline/MarkerEndAddition");
const wxString Config::sPathTimelineSnapClips("/Timeline/SnapClips");
const wxString Config::sPathTimelineSnapCursor("/Timeline/SnapCursor");
const wxString Config::sPathTimelineStripFromClipNames("/Timeline/StripFromClipNames");
const wxString Config::sPathTimelineLengthButtons("/Timeline/LengthButtonsTimesInMilliseconds");
const wxString Config::sPathVideoDefaultAlignment("/Video/DefaultAlignment");
const wxString Config::sPathVideoDefaultFrameRate("/Video/DefaultFrameRate");
const wxString Config::sPathVideoDefaultHeight("/Video/DefaultHeight");
const wxString Config::sPathVideoDefaultScaling("/Video/DefaultScaling");
const wxString Config::sPathVideoDefaultWidth("/Video/DefaultWidth");
const wxString Config::sPathVideoOverruleFourCC("/Video/FourCC");
const wxString Config::sPathVideoShowBoundingBox("/Video/ShowBoundingBox");
const wxString Config::sPathWorkspaceLanguage("/Workspace/Language");
const wxString Config::sPathWorkspaceH("/Workspace/H");
const wxString Config::sPathWorkspaceMaximized("/Workspace/Maximized");
const wxString Config::sPathWorkspacePerspectiveName("/Workspace/PerspectiveName");
const wxString Config::sPathWorkspacePerspectiveSaved("/Workspace/PerspectiveSaved");
const wxString Config::sPathWorkspaceW("/Workspace/W");
const wxString Config::sPathWorkspaceX("/Workspace/X");
const wxString Config::sPathWorkspaceY("/Workspace/Y");
const wxString Config::sPathWorkspacePerspectiveCurrent("/Workspace/PerspectiveCurrent");

//////////////////////////////////////////////////////////////////////////
// HELPER METHDOS
//////////////////////////////////////////////////////////////////////////

void Config::indexEntries()
{
    wxString key;
    wxString value;
    long cookie{ 88 };
    bool cont{ GetFirstEntry(key, cookie) };
    while (cont)
    {
        bool ok{ Read(key, &value) };
        mCache[GetPath() + "/" + key] = value;
        cont = GetNextEntry(key, cookie);
    }

    wxString group;
    cont = GetFirstGroup(group, cookie);
    while (cont)
    {
        wxString path{ GetPath() };
        SetPath(path + "/" + group);
        indexEntries();
        SetPath(path);
        cont = GetNextGroup(group, cookie);
    }
}

void Config::updateCache()
{
    ASSERT(wxThread::IsMain());
    mCache.clear();
    SetPath("");
    indexEntries();
    VAR_ERROR(mCache);
}

