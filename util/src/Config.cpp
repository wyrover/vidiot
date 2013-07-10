#include "Config.h"

#include <wx/confbase.h> // All config handling must be done via this file, for thread safety
#include <wx/config.h> // All config handling must be done via this file, for thread safety
#include "Enums.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "UtilInitAvcodec.h"

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
    // Initialize config object. Will be destructed by wxWidgets at the end of the application
    wxString ConfigFile(getFileName());
    VAR_ERROR(ConfigFile);
    wxConfigBase::Set(new wxFileConfig(applicationName, vendorName, ConfigFile));
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
    setDefault(Config::sPathDebugMaxRenderLength, 0); // Per default, render all
    setDefault(Config::sPathDebugShowCrashMenu, false);
    setDefault(Config::sPathDefaultAudioChannels, 2);
    setDefault(Config::sPathDefaultAudioSampleRate, 44100);
    setDefault(Config::sPathDefaultExtension, "avi");
    setDefault(Config::sPathDefaultFrameRate, "25");
    setDefault(Config::sPathDefaultTransitionLength, 24);
    setDefault(Config::sPathDefaultVideoAlignment, model::VideoAlignment_toString(model::VideoAlignmentCenter).c_str());
    setDefault(Config::sPathDefaultVideoHeight, 576);
    setDefault(Config::sPathDefaultVideoScaling, model::VideoScaling_toString(model::VideoScalingFitToFill).c_str());
    setDefault(Config::sPathDefaultVideoWidth, 720);
    setDefault(Config::sPathLastOpened, "");
    setDefault(Config::sPathLogLevel, LogLevel_toString(LogWarning).c_str());
    setDefault(Config::sPathLogLevelAvcodec, Avcodec::getDefaultLogLevel());
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
    setDefault(Config::sPathWindowMaximized, false);
    setDefault(Config::sPathWindowW, -1);
    setDefault(Config::sPathWindowH, -1);
    setDefault(Config::sPathWindowX, -1);
    setDefault(Config::sPathWindowY, -1);
    setDefault(Config::sPathWorkspacePerspectiveCurrent,"");

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

    // todo got crash when moving. Config::writelong failed in the window::onmove:
//    08-07-2013 22:45:21.059 ERROR   t@0ad0 D:/Vidiot/vidiot_trunk/util/src/Config.cpp(64) Config::init [ConfigFile=D:\Vidiot\Build\MSVC\main\RelWithDebInfo\Vidiot.ini]
//08-07-2013 22:45:29.358 WX      t@0ad0 wxLOG_Error Failed to copy the file 'D:\Vidiot\Build\MSVC\main\RelWithDebInfo\Vid3F1.tmp' to 'D:\Vidiot\Build\MSVC\main\RelWithDebInfo\Vidiot.ini' (error 5: access is denied.)
//08-07-2013 22:45:29.374 WX      t@0ad0 wxLOG_Error File 'D:\Vidiot\Build\MSVC\main\RelWithDebInfo\Vid3F1.tmp' couldn't be renamed 'D:\Vidiot\Build\MSVC\main\RelWithDebInfo\Vidiot.ini' (error 5: access is denied.)
//08-07-2013 22:45:29.374 WX      t@0ad0 wxLOG_Error can't commit changes to file 'D:\Vidiot\Build\MSVC\main\RelWithDebInfo\Vidiot.ini' (error 5: access is denied.)
//08-07-2013 22:45:29.374 WX      t@0ad0 wxLOG_Error Failed to update user configuration file.
//08-07-2013 22:45:29.374 ASSERT  t@0ad0 D:/Vidiot/vidiot_trunk/util/src/Config.cpp(226) Config::WriteLong [ASSERT:result]
//08-07-2013 22:45:32.213 STACK   t@0ad0
//  0 0108D98D      445  d:\vidiot\vidiot_trunk\util\src\utillog.cpp(264): LogVar::~LogVar
//  1 01090DA6      246  d:\vidiot\vidiot_trunk\util\src\config.cpp(228): Config::WriteLong
//  2 010E23D5       69  d:\vidiot\vidiot_trunk\gui\src\window.cpp(452): gui::Window::onMove
//  3 014E0F00       16  d:\vidiot\wxwidgets_trunk\src\common\appbase.cpp(602): wxAppConsoleBase::HandleEvent
//  4 014E0F3D       45  d:\vidiot\wxwidgets_trunk\src\common\appbase.cpp(613): wxAppConsoleBase::CallEventHandler
//  5 014F467B      139  d:\vidiot\wxwidgets_trunk\src\common\event.cpp(1744): wxEvtHandler::SearchDynamicEventTable
//  6 014F50E3       35  d:\vidiot\wxwidgets_trunk\src\common\event.cpp(1578): wxEvtHandler::TryHereOnly
//  7 014F5289       89  d:\vidiot\wxwidgets_trunk\src\common\event.cpp(1484): wxEvtHandler::ProcessEvent
//  8 014F43DF       79  d:\vidiot\wxwidgets_trunk\src\common\event.cpp(1547): wxEvtHandler::DoTryChain
//  9 014F5314      228  d:\vidiot\wxwidgets_trunk\src\common\event.cpp(1488): wxEvtHandler::ProcessEvent
// 10 014F448B       59  d:\vidiot\wxwidgets_trunk\src\common\event.cpp(1640): wxEvtHandler::SafelyProcessEvent
// 11 01374BAF      111  d:\vidiot\wxwidgets_trunk\src\msw\window.cpp(5079): wxWindow::HandleMove
// 12 0137B8FE      238  d:\vidiot\wxwidgets_trunk\src\msw\window.cpp(2758): wxWindow::MSWHandleMessage
// 13 0136E704       36  d:\vidiot\wxwidgets_trunk\src\msw\window.cpp(3614): wxWindow::MSWWindowProc
// 14 0138C390      288  d:\vidiot\wxwidgets_trunk\src\msw\toplevel.cpp(467): wxTopLevelWindowMSW::MSWWindowProc
// 15 013DFDAA      106  d:\vidiot\wxwidgets_trunk\src\msw\frame.cpp(884): wxFrame::MSWWindowProc
// 16 01376E0F      143  d:\vidiot\wxwidgets_trunk\src\msw\window.cpp(2703): wxWndProc
// 17 7708C4E7      463  gapfnScSendMessage
// 18 7708C5E7      719  gapfnScSendMessage
// 19 7707B0CC      466  GetMenuItemInfoW
// 20 77085138      187  DefWindowProcW
// 21 741F1E60        0
// 22 741F1F20        0
// 23 77085FC3      510  SetPropW
// 24 01373E8F       95  d:\vidiot\wxwidgets_trunk\src\msw\window.cpp(2278): wxWindow::MSWDefWindowProc
// 25 0136E71A       58  d:\vidiot\wxwidgets_trunk\src\msw\window.cpp(3620): wxWindow::MSWWindowProc
// 26 0138C390      288  d:\vidiot\wxwidgets_trunk\src\msw\toplevel.cpp(467): wxTopLevelWindowMSW::MSWWindowProc
// 27 013DFDAA      106  d:\vidiot\wxwidgets_trunk\src\msw\frame.cpp(884): wxFrame::MSWWindowProc
// 28 01376E0F      143  d:\vidiot\wxwidgets_trunk\src\msw\window.cpp(2703): wxWndProc
// 29 7708C4E7      463  gapfnScSendMessage
// 30 77085F9F      474  SetPropW
// 31 77084F0E      253  GetScrollBarInfo
// 32 7707FE58      205  GetSystemMenu
// 33 77396FCE       46  KiUserCallbackDispatcher
// 34 77085138      187  DefWindowProcW
// 35 741F5EEF     2021  Ordinal45
// 36 7420D128      322  SetWindowThemeAttribute
// 37 741F1E09        0
// 38 741F1F20        0
// 39 77085FC3      510  SetPropW
// 40 01373E8F       95  d:\vidiot\wxwidgets_trunk\src\msw\window.cpp(2278): wxWindow::MSWDefWindowProc
// 41 0136E71A       58  d:\vidiot\wxwidgets_trunk\src\msw\window.cpp(3620): wxWindow::MSWWindowProc
// 42 0138C390      288  d:\vidiot\wxwidgets_trunk\src\msw\toplevel.cpp(467): wxTopLevelWindowMSW::MSWWindowProc
// 43 013DFDAA      106  d:\vidiot\wxwidgets_trunk\src\msw\frame.cpp(884): wxFrame::MSWWindowProc
// 44 01376E0F      143  d:\vidiot\wxwidgets_trunk\src\msw\window.cpp(2703): wxWndProc
// 45 7708C4E7      463  gapfnScSendMessage
// 46 77085F9F      474  SetPropW
// 47 77084F0E      253  GetScrollBarInfo
// 48 77084F7D      364  GetScrollBarInfo
// 49 77396FCE       46  KiUserCallbackDispatcher
// 50 77085138      187  DefWindowProcW
// 51 741F5EEF     2021  Ordinal45
// 52 7420D175      399  SetWindowThemeAttribute
// 53 741F1E09        0
// 54 741F1F20        0
// 55 77085FC3      510  SetPropW
// 56 01373E8F       95  d:\vidiot\wxwidgets_trunk\src\msw\window.cpp(2278): wxWindow::MSWDefWindowProc
// 57 0136E71A       58  d:\vidiot\wxwidgets_trunk\src\msw\window.cpp(3620): wxWindow::MSWWindowProc
// 58 0138C390      288  d:\vidiot\wxwidgets_trunk\src\msw\toplevel.cpp(467): wxTopLevelWindowMSW::MSWWindowProc
// 59 013DFDAA      106  d:\vidiot\wxwidgets_trunk\src\msw\frame.cpp(884): wxFrame::MSWWindowProc
// 60 01376E0F      143  d:\vidiot\wxwidgets_trunk\src\msw\window.cpp(2703): wxWndProc
// 61 7708C4E7      463  gapfnScSendMessage
// 62 7708C5E7      719  gapfnScSendMessage
// 63 7708CC19     2305  gapfnScSendMessage
// 64 7708CC70       15  DispatchMessageW
// 65 770841EB      231  IsDialogMessageW
// 66 01374011      289  d:\vidiot\wxwidgets_trunk\src\msw\window.cpp(2487): wxWindow::MSWProcessMessage
// 67 0143494D      285  d:\vidiot\wxwidgets_trunk\src\msw\evtloop.cpp(149): wxGUIEventLoop::PreProcessMessage
// 68 014341CF       15  d:\vidiot\wxwidgets_trunk\src\msw\evtloop.cpp(167): wxGUIEventLoop::ProcessMessage
// 69 01434687      439  d:\vidiot\wxwidgets_trunk\src\msw\evtloop.cpp(233): wxGUIEventLoop::Dispatch
// 70 015638FD      109  d:\vidiot\wxwidgets_trunk\src\common\evtloopcmn.cpp(203): wxEventLoopManual::DoRun
// 71 01563A83      131  d:\vidiot\wxwidgets_trunk\src\common\evtloopcmn.cpp(79): wxEventLoopBase::Run
// 72 014E3F39      105  d:\vidiot\wxwidgets_trunk\src\common\appbase.cpp(324): wxAppConsoleBase::MainLoop
// 73 010D9197      119  d:\vidiot\vidiot_trunk\gui\src\application.cpp(177): gui::Application::OnRun
// 74 010496B8      184  d:\vidiot\vidiot_trunk\main\src\main.cpp(68): WinMain
// 75 0168AC5A      336  f:\dd\vctools\crt_bld\self_x86\crt\src\crtexe.c(547): __tmainCRTStartup
// 76 7598ED6C       18  BaseThreadInitThunk
// 77 773B377B      239  RtlInitializeExceptionChain
// 78 773B374E      194  RtlInitializeExceptionChain
//
//END CLOSE

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

void Config::WorkspacePerspectives::add(wxString name, wxString perspective)
{
    Config::Perspectives perspectives = get();
    perspectives[name] = perspective;
    save(perspectives);
}

void Config::WorkspacePerspectives::remove(wxString name)
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

void Config::WorkspacePerspectives::save(Perspectives perspectives)
{
    removeAll();
    int i = 1;
    BOOST_FOREACH( Config::Perspectives::value_type name_perspective, perspectives )
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

const wxString Config::sPathAutoLoadEnabled             ("/Project/AutoLoad/Enabled");
const wxString Config::sPathDebugMaxRenderLength        ("/Debug/MaxRenderLength");
const wxString Config::sPathDebugShowCrashMenu          ("/Debug/ShowCrashMenu");
const wxString Config::sPathDefaultAudioChannels        ("/Audio/DefaultNumberOfChannels");
const wxString Config::sPathDefaultAudioSampleRate      ("/Audio/DefaultSampleRate");
const wxString Config::sPathDefaultExtension            ("/File/DefaultExtension");
const wxString Config::sPathDefaultFrameRate            ("/Video/DefaultFrameRate");
const wxString Config::sPathDefaultTransitionLength     ("/Timeline/DefaultTransitionLength");
const wxString Config::sPathDefaultVideoAlignment       ("/Video/DefaultVideoAlignment");
const wxString Config::sPathDefaultVideoHeight          ("/Video/DefaultHeight");
const wxString Config::sPathDefaultVideoScaling         ("/Video/DefaultVideoScaling");
const wxString Config::sPathDefaultVideoWidth           ("/Video/DefaultWidth");
const wxString Config::sPathLastOpened                  ("/Project/LastOpened");
const wxString Config::sPathLogLevel                    ("/Debug/LogLevel");
const wxString Config::sPathLogLevelAvcodec             ("/Debug/LogLevelAvcodec");
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