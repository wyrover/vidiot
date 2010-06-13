#include "GuiOptions.h"

#include "wxInclude.h"
#include <wx/wfstream.h>
#include <wx/app.h>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <wx/sizer.h>
#include <wx/bookctrl.h>
#include <wx/radiobox.h>
#include <wx/spinctrl.h>
#include <wx/config.h>
#include <wx/fileconf.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/utils.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include "GuiMain.h"
#include "UtilLog.h"
#include "UtilLogAvcodec.h"

wxString GuiOptions::sConfigFile("");

//////////////////////////////////////////////////////////////////////////
// HELPER MACROS
//////////////////////////////////////////////////////////////////////////

#define GETPERSISTEDENUM(name,default)      name ## _fromString(std::string(wxConfigBase::Get()->Read(sPath ## name, name ## _toString( default )).mb_str()))
#define GETPERSISTEDSTRING(name,default)    wxConfigBase::Get()->Read(sPath ## name, default )
#define GETPERSISTEDBOOL(name,default)      wxConfigBase::Get()->ReadBool(sPath ## name, default)

//////////////////////////////////////////////////////////////////////////
// HELPER PATH VARIABLES
//////////////////////////////////////////////////////////////////////////

static const wxString sPathAutoLoadEnabled      ("/Project/AutoLoad/Enabled");
static const wxString sPathAutoLoadFilename     ("/Project/AutoLoad/Filename");
static const wxString sPathLastOpened           ("/Project/LastOpened");
static const wxString sPathLogLevel             ("/Debug/LogLevel");
static const wxString sPathLogFile              ("/Debug/LogFile");
static const wxString sPathFrameRate            ("/Video/FrameRate");

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

/**
 * This distributes the current set of options to the various other components.
 * Done initially, and after Tools->Options->Ok.
 * May not log as it is also called in the beginning of GuiMain::OnInit(), 
 * before the logging is initialized.
 */
void distributeOptions()
{
    Log::SetReportingLevel(GETPERSISTEDENUM(LogLevel,logINFO));
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
    return GETPERSISTEDSTRING(LogFile,defaultLogFile.GetFullPath());
}

// static 
wxString GuiOptions::getOptionsFileName()
{
    return sConfigFile;
}

// static
boost::optional<wxString> GuiOptions::GetAutoLoad()
{
    boost::optional<wxString> result;

    if (GETPERSISTEDBOOL(AutoLoadEnabled,false))
    {
        wxString filename = GETPERSISTEDSTRING(AutoLoadFilename,"");
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
    wxString frs = GETPERSISTEDSTRING(FrameRate,"");
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
        mLoadLast->SetValue(GETPERSISTEDBOOL(AutoLoadEnabled,false));
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
        switch (GETPERSISTEDENUM(LogLevel,logINFO))
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

        bool newLoadLast = mLoadLast->IsChecked();
        result = wxConfigBase::Get()->Write(sPathAutoLoadEnabled,newLoadLast);

        LogLevel newLogLevel = static_cast<LogLevel>(reinterpret_cast<int>(mLogLevel->GetClientData(mLogLevel->GetSelection())));
        result = wxConfigBase::Get()->Write(sPathLogLevel,LogLevel_toString(newLogLevel).c_str());

        wxString newFrameRate = boost::get<0>(sPossibleFrameRates[mFrameRate->GetSelection()]);
        result = wxConfigBase::Get()->Write(sPathFrameRate,newFrameRate);
        
        wxConfigBase::Get()->Flush();

        distributeOptions();
    }
}
