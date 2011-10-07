#include "Config.h"

#include <wx/fileconf.h>
#include <wx/filename.h>
#include "UtilLog.h"
#include "UtilInitAvcodec.h"

namespace gui {

wxString Config::sFileName("");

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

// static
void Config::init(wxString applicationName, wxString vendorName, bool inTestMode)
{
    // Initialize config object. Will be destructed by wxWidgets at the end of the application
    // This method ensures that the .ini file is created in the current working directory
    // which enables having multiple executables with multiple settings.
    sFileName = wxFileName(wxFileName::GetCwd(), applicationName + ".ini").GetFullPath();
    wxConfigBase::Set(new wxFileConfig(applicationName, vendorName, sFileName));
    wxConfigBase::Get()->Write(Config::sPathTest, inTestMode);
    
    Log::setReportingLevel(LogLevel_fromString(std::string(wxConfigBase::Get()->Read(Config::sPathLogLevel,"logINFO").mb_str())));
    if (inTestMode)
    {
        // Specific stuff for tests
        Log::setReportingLevel(logWARNING);
        // 
        // Log::setReportingLevel(logDEBUG); // (Set to warning for performance reasons)
    }
    Avcodec::configureLog();
}

wxString Config::getFileName()
{
    return sFileName;
}

//////////////////////////////////////////////////////////////////////////
// CONFIG PATHS
//////////////////////////////////////////////////////////////////////////

const wxString Config::sPathSnapClips               ("/View/SnapClips");
const wxString Config::sPathSnapCursor              ("/View/SnapCursor");
const wxString Config::sPathAutoLoadEnabled         ("/Project/AutoLoad/Enabled");
const wxString Config::sPathLastOpened              ("/Project/LastOpened");
const wxString Config::sPathLogLevel                ("/Debug/LogLevel");
const wxString Config::sPathShowDebugInfoOnWidgets  ("/Debug/Show");
const wxString Config::sPathTest                    ("/Debug/Test");
const wxString Config::sPathFrameRate               ("/Video/FrameRate");
const wxString Config::sPathMarkerBeginAddition     ("/Timeline/MarkerBeginAddition");
const wxString Config::sPathMarkerEndAddition       ("/Timeline/MarkerEndAddition");
const wxString Config::sPathStrip                   ("/Timeline/Strip");

} // namespace
