#include "GuiOptions.h"

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
#include <wx/utils.h>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include "GuiMain.h"
#include "UtilLog.h"
#include "UtilLogAvcodec.h"

namespace gui {

wxString    GuiOptions::sConfigFile("");
bool        GuiOptions::sShowDebugInfoOnWidgets = false;

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
void GuiOptions::distributeOptions()
{
    sShowDebugInfoOnWidgets = GETBOOL(sPathShowDebugInfoOnWidgets, false);
    Log::SetReportingLevel(GETENUM(sPathLogLevel,LogLevel,logINFO));
    Log::SetLogFile(std::string(GuiOptions::getLogFileName()));
    Avcodec::configureLog();
}

// static
void GuiOptions::init()
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
wxString GuiOptions::getLogFileName()
{
    wxString defaultFileName(wxGetApp().GetAppName());
    defaultFileName << "_" << wxGetProcessId() << ".log";
    wxFileName defaultLogFile(wxStandardPaths::Get().GetTempDir(),defaultFileName);
    return GETSTRING(sPathLogFile,defaultLogFile.GetFullPath());
}

// static 
wxString GuiOptions::getOptionsFileName()
{
    return sConfigFile;
}

// static
bool GuiOptions::getShowDebugInfoOnWidgets()
{
    return sShowDebugInfoOnWidgets;
}

// static
boost::optional<wxString> GuiOptions::GetAutoLoad()
{
    boost::optional<wxString> result;

    if (GETBOOL(sPathAutoLoadEnabled,false))
    {
        wxString filename = GETSTRING(sPathAutoLoadFilename,"");
        if (boost::filesystem::exists(boost::filesystem::path(filename)))
        {
            result.reset(filename);
        }
    }
    return result;
}

// static
void GuiOptions::SetAutoLoadFilename(wxString filename)
{
    wxConfigBase::Get()->Write(sPathAutoLoadFilename,filename);
    wxConfigBase::Get()->Flush();
}

model::FrameRate GuiOptions::getDefaultFrameRate()
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
double GuiOptions::getMarkerBeginAddition()
{
    return GETDOUBLE(sPathMarkerBeginAddition,0);
}

// static
double GuiOptions::getMarkerEndAddition()
{
    return GETDOUBLE(sPathMarkerEndAddition,0);
}

//////////////////////////////////////////////////////////////////////////
// CONFIGURATION DIALOG
//////////////////////////////////////////////////////////////////////////

GuiOptions::GuiOptions(wxWindow* win)
:   wxPropertySheetDialog(win, wxID_ANY, _("Preferences"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    // Extra scoping below is for easier copy/pasting for new tabs

    //////////////////////////////////////////////////////////////////////////
    // GENERAL
    //////////////////////////////////////////////////////////////////////////
    
    {
        // Tab: General
        wxPanel* panel = new wxPanel(GetBookCtrl(), wxID_ANY);
        GetBookCtrl()->AddPage(panel, _("General"), true);
        wxBoxSizer *topSizer = new wxBoxSizer( wxVERTICAL );
        panel->SetSizerAndFit(topSizer);

        // Box: Start 
        wxStaticBox*    staticbox       = new wxStaticBox(panel, wxID_ANY, _("Startup"));
        wxBoxSizer*     staticbox_sizer = new wxStaticBoxSizer( staticbox, wxVERTICAL );
        topSizer->Add(staticbox_sizer, 1, wxGROW|wxALIGN_CENTRE|wxALL, 5 );

        // Entry: Load last project on startup
        wxBoxSizer*     hSizer          = new wxBoxSizer( wxHORIZONTAL );
        staticbox_sizer->Add(hSizer, 0, wxGROW|wxALL, 5);
        mLoadLast = new wxCheckBox(panel, wxID_ANY, _("&Load last project on startup"), wxDefaultPosition, wxDefaultSize);
        mLoadLast->SetValue(GETBOOL(sPathAutoLoadEnabled,false));
        hSizer->Add(mLoadLast, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
    }

    //////////////////////////////////////////////////////////////////////////
    // VIDEO
    //////////////////////////////////////////////////////////////////////////

    {
        // Tab: Video
        wxPanel* panel = new wxPanel(GetBookCtrl(), wxID_ANY);
        GetBookCtrl()->AddPage(panel, _("Video"), false);
        wxBoxSizer *topSizer = new wxBoxSizer( wxVERTICAL );
        panel->SetSizerAndFit(topSizer);

        // Box: Framerate
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
        mFrameRate = new wxRadioBox(panel, wxID_ANY, wxT("Framerate for new projects"),wxPoint(10,10), wxDefaultSize, choices, 1, wxRA_SPECIFY_COLS );
        mFrameRate->SetSelection(selection);
        topSizer->Add(mFrameRate, 1, wxGROW|wxALIGN_CENTRE|wxALL, 5 );
    }

    //////////////////////////////////////////////////////////////////////////
    // TIMELINE
    //////////////////////////////////////////////////////////////////////////

    {
        // Tab: Timeline
        wxPanel* panel = new wxPanel(GetBookCtrl(), wxID_ANY);
        GetBookCtrl()->AddPage(panel, _("Timeline"), false);
        wxBoxSizer *topSizer = new wxBoxSizer( wxVERTICAL );
        panel->SetSizerAndFit(topSizer);

        // Box: Start 
        wxStaticBox*    staticbox       = new wxStaticBox(panel, wxID_ANY, _("Marking selection"));
        wxBoxSizer*     staticbox_sizer = new wxStaticBoxSizer( staticbox, wxVERTICAL );
        topSizer->Add(staticbox_sizer, 1, wxGROW|wxALIGN_CENTRE|wxALL, 5 );

        // Box: Marking selection while playing

        // Entry: Begin addition
        {
            wxBoxSizer*     hSizer          = new wxBoxSizer( wxHORIZONTAL );
            staticbox_sizer->Add(hSizer, 0, wxGROW|wxALL, 5);
            double initial = getMarkerBeginAddition();
            mMarkerBeginAddition = new wxSpinCtrlDouble(panel, wxID_ANY, wxString::Format("%1.1f", initial), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, -10, 10, initial, 0.1);
            hSizer->Add(new wxStaticText(panel, wxID_ANY, _("Begin marker expansion/contraction (s).")), 0, wxLEFT|wxALIGN_CENTER_VERTICAL, 5);
            hSizer->Add(5, 5, 1, wxALL, 0);
            hSizer->Add(mMarkerBeginAddition, 0, wxRIGHT|wxALIGN_CENTER_VERTICAL, 5);
        }

        // Entry: End addition
        {
            wxBoxSizer*     hSizer          = new wxBoxSizer( wxHORIZONTAL );
            staticbox_sizer->Add(hSizer, 0, wxGROW|wxALL, 5);
            double initial = getMarkerEndAddition();
            mMarkerEndAddition = new wxSpinCtrlDouble(panel, wxID_ANY, wxString::Format("%1.1f", initial), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, -10, 10, initial, 0.1);
            hSizer->Add(new wxStaticText(panel, wxID_ANY, _("End marker expansion/contraction (s).")), 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
            hSizer->Add(5, 5, 1, wxALL, 0);
            hSizer->Add(mMarkerEndAddition, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // DEBUG
    //////////////////////////////////////////////////////////////////////////

    {
        // Tab: Debug
        wxPanel* panel = new wxPanel(GetBookCtrl(), wxID_ANY);
        GetBookCtrl()->AddPage(panel, _("Debug"), false);
        wxBoxSizer *topSizer = new wxBoxSizer( wxVERTICAL );
        panel->SetSizerAndFit(topSizer);

        // Box: Debug
        wxStaticBox*    staticbox       = new wxStaticBox(panel, wxID_ANY, _("Start"));
        wxBoxSizer*     staticbox_sizer = new wxStaticBoxSizer( staticbox, wxVERTICAL );
        topSizer->Add(staticbox_sizer, 1, wxGROW|wxALIGN_CENTRE|wxALL, 5 );

        // Entry: Log level
        wxBoxSizer*     hSizer          = new wxBoxSizer( wxHORIZONTAL );
        staticbox_sizer->Add(hSizer, 0, wxGROW|wxALL, 5);
        mLogLevel = new wxChoice(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize);
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
        hSizer->Add(new wxStaticText(panel, wxID_ANY, _("&Log level:")), 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
        hSizer->Add(5, 5, 1, wxALL, 0);
        hSizer->Add(mLogLevel, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);

        // Entry: Show debug info on widgets
        wxBoxSizer*     hSizerShow          = new wxBoxSizer( wxHORIZONTAL );
        staticbox_sizer->Add(hSizerShow, 0, wxGROW|wxALL, 5);
        mShowDebugInfoOnWidgets = new wxCheckBox(panel, wxID_ANY, _("&Show debug info on widgets"), wxDefaultPosition, wxDefaultSize);
        mShowDebugInfoOnWidgets->SetValue(GETBOOL(sPathShowDebugInfoOnWidgets,false));
        hSizerShow->Add(mShowDebugInfoOnWidgets, 0, wxALL|wxALIGN_CENTER_VERTICAL, 5);
    }

    SetExtraStyle(wxDIALOG_EX_CONTEXTHELP|wxWS_EX_VALIDATE_RECURSIVELY);
    CreateButtons(wxOK | wxCANCEL);

    LayoutDialog();
}

GuiOptions::~GuiOptions()
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

        wxConfigBase::Get()->Flush();

        distributeOptions();
    }
}

} // namespace
