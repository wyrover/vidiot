#include "Config.h"

#include "Enums.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "UtilInitAvcodec.h"

wxString Config::sFileName("");
bool Config::sShowDebugInfo(false);
boost::mutex Config::sMutex;

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

template <class T>
T readWithoutDefault(wxString path, boost::mutex& mutex)
{
    boost::mutex::scoped_lock lock(mutex);
    T result = T();
    T dummy = T();
    bool found = wxConfigBase::Get()->Read(path, &result, dummy);
    ASSERT(found)(path);
    return result;
}

// static
void Config::init(wxString applicationName, wxString vendorName, bool inCxxTestMode)
{
    // Initialize config object. Will be destructed by wxWidgets at the end of the application
    // This method ensures that the .ini file is created in the current working directory
    // which enables having multiple executables with multiple settings.
    sFileName = wxFileName(wxFileName::GetCwd(), applicationName + ".ini").GetFullPath();
    wxConfigBase::Set(new wxFileConfig(applicationName, vendorName, sFileName));
    wxConfigBase::Get()->Write(Config::sPathTest, inCxxTestMode);

    // Set all defaults here
    setDefault(Config::sPathAutoLoadEnabled, false);
    setDefault(Config::sPathDefaultTransitionLength, 24);
    setDefault(Config::sPathDefaultFrameRate, "25");
    setDefault(Config::sPathDefaultVideoWidth, 720);
    setDefault(Config::sPathDefaultVideoHeight, 576);
    setDefault(Config::sPathDefaultVideoScaling, model::VideoScaling_toString(model::VideoScalingFitToFill).c_str());
    setDefault(Config::sPathDefaultVideoAlignment, model::VideoAlignment_toString(model::VideoAlignmentCenter).c_str());
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
bool Config::ReadBool(const wxString& key)
{
    return readWithoutDefault<bool>(key,sMutex);
}

// static
long Config::ReadLong(const wxString& key)
{
    return readWithoutDefault<long>(key,sMutex);
}

// static
double Config::ReadDouble(const wxString& key)
{
    return readWithoutDefault<double>(key,sMutex);
}

// static
wxString Config::ReadString(const wxString& key)
{
    return readWithoutDefault<wxString>(key,sMutex);
}

// static
bool Config::getShowDebugInfo()
{
    return sShowDebugInfo;
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
const wxString Config::sPathDefaultFrameRate        ("/Video/DefaultFrameRate");
const wxString Config::sPathDefaultVideoWidth       ("/Video/DefaultWidth");
const wxString Config::sPathDefaultVideoHeight      ("/Video/DefaultHeight");
const wxString Config::sPathDefaultVideoScaling     ("/Video/DefaultVideoScaling");
const wxString Config::sPathDefaultVideoAlignment   ("/Video/DefaultVideoAlignment");
const wxString Config::sPathMarkerBeginAddition     ("/Timeline/MarkerBeginAddition");
const wxString Config::sPathMarkerEndAddition       ("/Timeline/MarkerEndAddition");
const wxString Config::sPathStrip                   ("/Timeline/Strip");
const wxString Config::sPathDefaultTransitionLength ("/Timeline/DefaultTransitionLength");