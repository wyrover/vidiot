#include "Options.h"

#include <wx/wfstream.h>
#include <wx/app.h>
#include <wx/sizer.h>
#include <wx/bookctrl.h>
#include <wx/radiobox.h>
#include <wx/spinctrl.h>
#include <wx/config.h>
#include <wx/fileconf.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/utils.h>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include "Main.h"
#include "UtilLog.h"
#include "UtilLogAvcodec.h"

namespace gui {

wxString    Options::sConfigFile("");
bool        Options::sShowDebugInfoOnWidgets = false;
wxString    Options::sStrip("");

//////////////////////////////////////////////////////////////////////////
// HELPER MACROS
//////////////////////////////////////////////////////////////////////////

#define GETENUM(name,enumtype,default)  enumtype ## _fromString(std::string(wxConfigBase::Get()->Read(name, enumtype ## _toString( default )).mb_str()))
#define GETSTRING(name,default)         wxConfigBase::Get()->Read(name, default)
#define GETBOOL(name,default)           wxConfigBase::Get()->ReadBool(name, default)
#define GETDOUBLE(name,default)         wxConfigBase::Get()->ReadDouble(name, default)

#define SETENUM(name,enumtype,value)    result = wxConfigBase::Get()->Write(name, enumtype ## _toString( value ).c_str()); ASSERT(result)
#define SETSTRING(name,value)           result = wxConfigBase::Get()->Write(name, value); ASSERT(result)
#define SETBOOL(name,value)             result = wxConfigBase::Get()->Write(name, value); ASSERT(result)
#define SETDOUBLE(name,value)           result = wxConfigBase::Get()->Write(name, value); ASSERT(result)

//////////////////////////////////////////////////////////////////////////
// HELPER PATH VARIABLES
//////////////////////////////////////////////////////////////////////////

static const wxString sPathAutoLoadEnabled          ("/Project/AutoLoad/Enabled");
static const wxString sPathAutoLoadFilename         ("/Project/AutoLoad/Filename");
static const wxString sPathLastOpened               ("/Project/LastOpened");
static const wxString sPathLogLevel                 ("/Debug/LogLevel");
static const wxString sPathLogFile                  ("/Debug/LogFile");
static const wxString sPathShowDebugInfoOnWidgets   ("/Debug/Show");
static const wxString sPathFrameRate                ("/Video/FrameRate");
static const wxString sPathMarkerBeginAddition      ("/Timeline/MarkerBeginAddition");
static const wxString sPathMarkerEndAddition        ("/Timeline/MarkerEndAddition");
static const wxString sPathStrip                    ("/Timeline/Strip");

//////////////////////////////////////////////////////////////////////////
// HELPER VARIABLES
//////////////////////////////////////////////////////////////////////////

typedef boost::tuple<wxString, model::FrameRate> FrameRateEntry; 
std::vector<FrameRateEntry> sPossibleFrameRates = boost::assign::tuple_list_of
    ( wxT("24.97"), model::framerate::s24p )
    ( wxT("25"),    model::framerate::s25p )
    ( wxT("29.97"), model::framerate::s30p );

//////////////////////////////////////////////////////////////////////////
// APPLICATION INITIALIZATION & CONFIGURATION
//////////////////////////////////////////////////////////////////////////

// static
void Options::distributeOptions()
{
    sShowDebugInfoOnWidgets = GETBOOL(sPathShowDebugInfoOnWidgets, false);
    Log::SetReportingLevel(GETENUM(sPathLogLevel,LogLevel,logINFO));
    Log::SetLogFile(std::string(Options::getLogFileName()));
    Avcodec::configureLog();
}

// static
void Options::init()
{
    // Initialize config object. Will be destructed by wxWidgets at the end of the application
    sConfigFile = wxFileName(wxFileName::GetCwd(),wxGetApp().GetAppName()+".ini").GetFullPath();
    wxConfigBase::Set(new wxFileConfig(wxGetApp().GetAppName(),wxGetApp().GetVendorName(),sConfigFile));
    distributeOptions();
}

//////////////////////////////////////////////////////////////////////////
// SETTERS & GETTERS
//////////////////////////////////////////////////////////////////////////

// static 
wxString Options::getLogFileName()
{
    wxString defaultFileName(wxGetApp().GetAppName());
    defaultFileName << "_" << wxGetProcessId() << ".log";
    wxFileName defaultLogFile(wxStandardPaths::Get().GetTempDir(),defaultFileName);
    return GETSTRING(sPathLogFile,defaultLogFile.GetFullPath());
}

// static 
wxString Options::getOptionsFileName()
{
    return sConfigFile;
}

// static
bool Options::getShowDebugInfoOnWidgets()
{
    return sShowDebugInfoOnWidgets;
}

// static
boost::optional<wxString> Options::GetAutoLoad()
{
    boost::optional<wxString> result;

    if (GETBOOL(sPathAutoLoadEnabled,false))
    {
        wxString filename = GETSTRING(sPathAutoLoadFilename,"");
        if (boost::filesystem::exists(boost::filesystem::path(filename.ToStdString())))
        {
            result.reset(filename);
        }
    }
    return result;
}

// static
void Options::SetAutoLoadFilename(wxString filename)
{
    wxConfigBase::Get()->Write(sPathAutoLoadFilename,filename);
    wxConfigBase::Get()->Flush();
}

model::FrameRate Options::getDefaultFrameRate()
{
    model::FrameRate fr = model::framerate::s25p;
    wxString frs = GETSTRING(sPathFrameRate,"");
    for (unsigned int i = 0; i < sPossibleFrameRates.size(); ++i)
    {
        if (frs.IsSameAs(boost::get<0>(sPossibleFrameRates[i])))
        {
            fr = boost::get<1>(sPossibleFrameRates[i]);
            break;
        }
    };

    return fr;
}

// static
double Options::getMarkerBeginAddition()
{
    return GETDOUBLE(sPathMarkerBeginAddition,0);
}

// static
double Options::getMarkerEndAddition()
{
    return GETDOUBLE(sPathMarkerEndAddition,0);
}

// static
wxString Options::getTimelineStrip()
{
    return sStrip;
}

//////////////////////////////////////////////////////////////////////////
// CONFIGURATION DIALOG
//////////////////////////////////////////////////////////////////////////

class OptionHelper
{
public:
    OptionHelper(Options* window)
        :   mWindow(window)
        ,   panel(0)
        ,   topSizer(0)
        ,   staticbox_sizer(0)
    {
    };
    void tab(const wxString& name)
    {
        panel = new wxPanel(mWindow->GetBookCtrl(), wxID_ANY);
        mWindow->GetBookCtrl()->AddPage(panel, name, true);
        topSizer = new wxBoxSizer( wxVERTICAL );
        panel->SetSizerAndFit(topSizer);
        staticbox_sizer = 0;
    }
    void box(const wxString& name)
    {
        ASSERT(panel);
        ASSERT(topSizer);
        wxStaticBox* staticbox = new wxStaticBox(panel, wxID_ANY, name);
        staticbox_sizer = new wxStaticBoxSizer( staticbox, wxVERTICAL );
        topSizer->Add(staticbox_sizer, 1, wxGROW|wxALIGN_CENTRE|wxALL, 5 );
    }
    void option(const wxString& name, wxWindow* widget)
    {
        ASSERT(staticbox_sizer);
        wxBoxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );
        staticbox_sizer->Add(hSizer, 0, wxGROW|wxLEFT|wxALL, 5);
        hSizer->Add(new wxStaticText(panel, wxID_ANY, name), 0, wxALL|wxALIGN_TOP, 5);
        hSizer->Add(5, 5, 1, wxALL, 0);
        hSizer->Add(widget, 0, wxRIGHT|wxALIGN_TOP, 5);
    }
    wxWindow* getParent()
    {
        ASSERT(panel);
        return panel;
    }
private:
    Options* mWindow;
    wxPanel* panel;                 ///< tab:The topmost widget
    wxBoxSizer* topSizer;           ///< tab:Sizer for panel
    wxBoxSizer* staticbox_sizer;    ///< box:Sizer for current box
};

Options::Options(wxWindow* win)
:   wxPropertySheetDialog(win, wxID_ANY, _("Preferences"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    sStrip = GETSTRING(sPathStrip,_T(""));
    sShowDebugInfoOnWidgets = GETBOOL(sPathShowDebugInfoOnWidgets,false);
        
    OptionHelper add(this);

    //////////////////////////////////////////////////////////////////////////
 
    add.tab(_("General"));

    add.box(_("Startup"));

    mLoadLast = new wxCheckBox(add.getParent(), wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize);
    mLoadLast->SetValue(GETBOOL(sPathAutoLoadEnabled,false));
    add.option(_("Load last project on startup"), mLoadLast);

    //////////////////////////////////////////////////////////////////////////

    add.tab(_("Video"));

    add.box(_("New projects"));

    wxArrayString choices;
    unsigned int selection = 0;
    model::FrameRate currentFrameRate = getDefaultFrameRate();
    for (unsigned int i = 0; i < sPossibleFrameRates.size(); ++i)
    {
        choices.Add(boost::get<0>(sPossibleFrameRates[i]));
        if (currentFrameRate == boost::get<1>(sPossibleFrameRates[i]))
        {
            selection = i;
        }
    };
    mFrameRate = new wxRadioBox(add.getParent(), wxID_ANY, wxT(""),wxPoint(10,10), wxDefaultSize, choices, 1, wxRA_SPECIFY_COLS );
    mFrameRate->SetSelection(selection);
    add.option(_("Framerate for new projects"), mFrameRate);

    //////////////////////////////////////////////////////////////////////////

    add.tab(_("Timeline"));

    add.box(_("Marking selection"));

    double initial = getMarkerBeginAddition();
    mMarkerBeginAddition = new wxSpinCtrlDouble(add.getParent(), wxID_ANY, wxString::Format("%1.1f", initial), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, -10, 10, initial, 0.1);
    add.option(_("Begin marker expansion/contraction (s)."), mMarkerBeginAddition);

    initial = getMarkerEndAddition();
    mMarkerEndAddition = new wxSpinCtrlDouble(add.getParent(), wxID_ANY, wxString::Format("%1.1f", initial), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, -10, 10, initial, 0.1);
    add.option(_("End marker expansion/contraction (s)."), mMarkerEndAddition);

    add.box(_("Clips"));
    mStrip = new wxTextCtrl(add.getParent(), wxID_ANY, sStrip);
    add.option(_("Text to remove once from clip names (requires restart)"), mStrip);

    //////////////////////////////////////////////////////////////////////////

    add.tab(_("Debug"));

    add.box(_("Logging"));

    mLogLevel = new wxChoice(add.getParent(), wxID_ANY);
    mLogLevel->Append(_("Error"),      reinterpret_cast<void*>(logERROR));
    mLogLevel->Append(_("Warning"),    reinterpret_cast<void*>(logWARNING));
    mLogLevel->Append(_("Info"),       reinterpret_cast<void*>(logINFO));
    mLogLevel->Append(_("Debug"),      reinterpret_cast<void*>(logDEBUG));
    mLogLevel->Append(_("Video"),      reinterpret_cast<void*>(logVIDEO));
    mLogLevel->Append(_("Audio"),      reinterpret_cast<void*>(logAUDIO));
    mLogLevel->Append(_("Detailed"),   reinterpret_cast<void*>(logDETAIL));
    switch (GETENUM(sPathLogLevel,LogLevel,logINFO))
    {
    case logERROR:      mLogLevel->SetSelection(0); break;
    case logWARNING:    mLogLevel->SetSelection(1); break;
    case logINFO:       mLogLevel->SetSelection(2); break;
    case logDEBUG:      mLogLevel->SetSelection(3); break;
    case logVIDEO:      mLogLevel->SetSelection(4); break;
    case logAUDIO:      mLogLevel->SetSelection(5); break;
    case logDETAIL:     mLogLevel->SetSelection(6); break;
    }
    add.option(_("Log level"), mLogLevel);

    mShowDebugInfoOnWidgets = new wxCheckBox(add.getParent(), wxID_ANY, _T(""));
    mShowDebugInfoOnWidgets->SetValue(sShowDebugInfoOnWidgets);
    add.option(_("Show debug info on widgets (requires restart)"), mShowDebugInfoOnWidgets);

    //////////////////////////////////////////////////////////////////////////

    SetExtraStyle(wxDIALOG_EX_CONTEXTHELP|wxWS_EX_VALIDATE_RECURSIVELY);
    CreateButtons(wxOK | wxCANCEL);

    LayoutDialog();
}

Options::~Options()
{
    if (GetReturnCode() == GetAffirmativeId())
    {
        bool result;

        SETBOOL(    sPathAutoLoadEnabled,           mLoadLast->IsChecked());
        SETENUM(    sPathLogLevel, LogLevel,        static_cast<LogLevel>(reinterpret_cast<int>(mLogLevel->GetClientData(mLogLevel->GetSelection()))));
        SETBOOL(    sPathShowDebugInfoOnWidgets,    mShowDebugInfoOnWidgets->IsChecked());
        SETSTRING(  sPathFrameRate,                 boost::get<0>(sPossibleFrameRates[mFrameRate->GetSelection()]));
        SETDOUBLE(  sPathMarkerBeginAddition,       mMarkerBeginAddition->GetValue());
        SETDOUBLE(  sPathMarkerEndAddition,         mMarkerEndAddition->GetValue());
        SETSTRING(  sPathStrip,                     mStrip->GetValue());

        wxConfigBase::Get()->Flush();

        distributeOptions();
    }
}

} // namespace
