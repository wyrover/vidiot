#include "Config.h"

#include <wx/confbase.h> // All config handling must be done via this file, for thread safety
#include <wx/config.h> // All config handling must be done via this file, for thread safety
#include "Enums.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "UtilInitAvcodec.h"

wxString Config::sFileName("");
bool Config::sShowDebugInfo(false);
boost::mutex Config::sMutex;
bool Config::sHold(false);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

template <class T>
void setDefault(wxString path, T value)
{
    if (!wxConfigBase::Get()->Exists(path))
    {
        wxConfigBase::Get()->Write(path, value);
    }
}

void checkLong(wxString path, long lowerbound, long upperbound)
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

void checkBool(wxString path)
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

// static
void Config::init(wxString applicationName, wxString vendorName, bool inCxxTestMode)
{
    wxString configdir = wxFileName::GetCwd();

    if (configdir.Contains("Program Files"))
    {
        // When running from "Program Files", store this file elsewhere to avoid being unable to write.
        configdir = wxStandardPaths::Get().GetUserConfigDir(); // Store in "C:\Users\<username>\AppData\Roaming\vidiot.ini"
    }

    // Initialize config object. Will be destructed by wxWidgets at the end of the application
    // This method ensures that the .ini file is created in the current working directory
    // which enables having multiple executables with multiple settings.
    sFileName = wxFileName(configdir, applicationName + ".ini").GetFullPath();

    wxConfigBase::Set(new wxFileConfig(applicationName, vendorName, sFileName));
    wxConfigBase::Get()->Write(Config::sPathTest, inCxxTestMode);

    // Check values, delete from config if incorrect
    checkBool(Config::sPathAutoLoadEnabled);
    checkLong(Config::sPathDefaultTransitionLength, 4, 10000);
    checkLong(Config::sPathDefaultFrameRate, 4, 10000);
    checkLong(Config::sPathDefaultVideoWidth, 10, 10000);
    checkLong(Config::sPathDefaultVideoHeight, 10, 10000);
    checkEnum(Config::sPathDefaultVideoScaling, model::VideoScaling);
    checkEnum(Config::sPathDefaultVideoAlignment, model::VideoAlignment);
    checkLong(Config::sPathDefaultAudioSampleRate, 100, 100000);
    checkLong(Config::sPathDefaultAudioChannels, 1, 2);
    checkEnum(Config::sPathLogLevel, LogLevel);
    checkLong(Config::sPathMarkerBeginAddition, 0, 10000);
    checkLong(Config::sPathMarkerEndAddition, 0, 10000);
    checkBool(Config::sPathShowDebugInfoOnWidgets);
    checkBool(Config::sPathSnapClips);
    checkBool(Config::sPathSnapCursor);
    checkBool(Config::sPathShowBoundingBox);
    checkLong(Config::sPathDebugMaxRenderLength, 0, 1000000);

    // Set all defaults here
    setDefault(Config::sPathAutoLoadEnabled, false);
    setDefault(Config::sPathDefaultTransitionLength, 24);
    setDefault(Config::sPathDefaultFrameRate, "25");
    setDefault(Config::sPathDefaultVideoWidth, 720);
    setDefault(Config::sPathDefaultVideoHeight, 576);
    setDefault(Config::sPathDefaultVideoScaling, model::VideoScaling_toString(model::VideoScalingFitToFill).c_str());
    setDefault(Config::sPathDefaultVideoAlignment, model::VideoAlignment_toString(model::VideoAlignmentCenter).c_str());
    setDefault(Config::sPathDefaultAudioSampleRate, 44100);
    setDefault(Config::sPathDefaultAudioChannels, 2);
    setDefault(Config::sPathLastOpened, "");
    setDefault(Config::sPathDefaultExtension, "avi");
    setDefault(Config::sPathLogLevel, LogLevel_toString(LogWarning).c_str());
    setDefault(Config::sPathLogLevelAvcodec, Avcodec::getDefaultLogLevel());
    setDefault(Config::sPathMarkerBeginAddition, 0);
    setDefault(Config::sPathMarkerEndAddition, 0);
    setDefault(Config::sPathShowDebugInfoOnWidgets, false);
    setDefault(Config::sPathSnapClips, true);
    setDefault(Config::sPathSnapCursor, true);
    setDefault(Config::sPathShowBoundingBox, true);
    setDefault(Config::sPathStrip, "scene'2010");
    setDefault(Config::sPathDebugMaxRenderLength, 0); // Per default, render all

    wxConfigBase::Get()->Flush();

    // Read cached values here
    Log::setReportingLevel(LogLevelConverter::readConfigValue(Config::sPathLogLevel));
    sShowDebugInfo = Config::ReadBool(Config::sPathShowDebugInfoOnWidgets);

    Avcodec::configureLog();
}

wxString Config::getFileName()
{
    return sFileName;
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
T readWithoutDefault(wxString path)
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
    if (!sHold)
    {
        result = wxConfigBase::Get()->Flush();
        ASSERT(result);
    }
}

// static
void Config::WriteLong(const wxString& key, long value)
{
    boost::mutex::scoped_lock lock(sMutex);
    bool result = wxConfigBase::Get()->Write(key, value);
    ASSERT(result);
    if (!sHold)
    {
        result = wxConfigBase::Get()->Flush();
        ASSERT(result);
    }
}

// static
void Config::WriteDouble(const wxString& key, double value)
{
    boost::mutex::scoped_lock lock(sMutex);
    bool result = wxConfigBase::Get()->Write(key, value);
    ASSERT(result);
    if (!sHold)
    {
        result = wxConfigBase::Get()->Flush();
        ASSERT(result);
    }
}

// static
void Config::WriteString(const wxString& key, wxString value)
{
    boost::mutex::scoped_lock lock(sMutex);
    bool result = wxConfigBase::Get()->Write(key, value);
    ASSERT(result);
    if (!sHold)
    {
        result = wxConfigBase::Get()->Flush();
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

const wxString Config::sPathSnapClips               ("/View/SnapClips");
const wxString Config::sPathSnapCursor              ("/View/SnapCursor");
const wxString Config::sPathShowBoundingBox         ("/View/BoundingBox");
const wxString Config::sPathAutoLoadEnabled         ("/Project/AutoLoad/Enabled");
const wxString Config::sPathLastOpened              ("/Project/LastOpened");
const wxString Config::sPathDefaultExtension        ("/File/DefaultExtension");
const wxString Config::sPathLogLevel                ("/Debug/LogLevel");
const wxString Config::sPathLogLevelAvcodec         ("/Debug/LogLevelAvcodec");
const wxString Config::sPathShowDebugInfoOnWidgets  ("/Debug/Show");
const wxString Config::sPathTest                    ("/Debug/Test");
const wxString Config::sPathDebugMaxRenderLength    ("/Debug/MaxRenderLength");
const wxString Config::sPathDefaultFrameRate        ("/Video/DefaultFrameRate");
const wxString Config::sPathDefaultVideoWidth       ("/Video/DefaultWidth");
const wxString Config::sPathDefaultVideoHeight      ("/Video/DefaultHeight");
const wxString Config::sPathDefaultVideoScaling     ("/Video/DefaultVideoScaling");
const wxString Config::sPathDefaultVideoAlignment   ("/Video/DefaultVideoAlignment");
const wxString Config::sPathOverruleFourCC          ("/Video/FourCC");
const wxString Config::sPathDefaultAudioSampleRate  ("/Audio/DefaultSampleRate");
const wxString Config::sPathDefaultAudioChannels    ("/Audio/DefaultNumberOfChannels");
const wxString Config::sPathMarkerBeginAddition     ("/Timeline/MarkerBeginAddition");
const wxString Config::sPathMarkerEndAddition       ("/Timeline/MarkerEndAddition");
const wxString Config::sPathStrip                   ("/Timeline/Strip");
const wxString Config::sPathDefaultTransitionLength ("/Timeline/DefaultTransitionLength");