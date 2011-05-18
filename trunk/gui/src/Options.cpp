#include "Options.h"

#include <wx/wfstream.h>
#include <wx/app.h>
#include <wx/sizer.h>
#include <wx/bookctrl.h>
#include <wx/radiobox.h>
#include <wx/spinctrl.h>
#include <wx/config.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include "UtilLog.h"
#include "UtilLogAvcodec.h"
#include "UtilFrameRate.h"
#include "Config.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Options::Options(wxWindow* win)
:   wxPropertySheetDialog(win, wxID_ANY, _("Preferences"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
,   mPanel(0)
,   mTopSizer(0)
,   mBoxSizer(0)
{
    //////////////////////////////////////////////////////////////////////////
 
    addtab(_("General"));

    addbox(_("Startup"));

    mLoadLast = new wxCheckBox(mPanel, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize);
    mLoadLast->SetValue(wxConfigBase::Get()->ReadBool(Config::sPathAutoLoadEnabled,false));
    addoption(_("Load last project on startup"), mLoadLast);

    //////////////////////////////////////////////////////////////////////////

    addtab(_("Video"));

    addbox(_("New projects"));

    wxArrayString choices;
    unsigned int selection = 0;
    wxString currentFrameRate = wxConfigBase::Get()->Read(Config::sPathFrameRate,"25");
    BOOST_FOREACH( FrameRate fr, framerate::getSupported() )
    {
        wxString frs = framerate::toString(fr);
        choices.Add(frs);
        if (currentFrameRate.IsSameAs(frs))
        {
            selection = choices.GetCount() - 1;
        }
    }
    mFrameRate = new wxRadioBox(mPanel, wxID_ANY, wxT(""),wxPoint(10,10), wxDefaultSize, choices, 1, wxRA_SPECIFY_COLS );
    mFrameRate->SetSelection(selection);
    addoption(_("Framerate for new projects"), mFrameRate);

    //////////////////////////////////////////////////////////////////////////

    addtab(_("Timeline"));

    addbox(_("Marking selection"));

    double initial = wxConfigBase::Get()->ReadDouble(Config::sPathMarkerBeginAddition, 0);
    mMarkerBeginAddition = new wxSpinCtrlDouble(mPanel, wxID_ANY, wxString::Format("%1.1f", initial), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, -10, 10, initial, 0.1);
    addoption(_("Begin marker expansion/contraction (s)."), mMarkerBeginAddition);

    initial = wxConfigBase::Get()->ReadDouble(Config::sPathMarkerEndAddition, 0);
    mMarkerEndAddition = new wxSpinCtrlDouble(mPanel, wxID_ANY, wxString::Format("%1.1f", initial), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, -10, 10, initial, 0.1);
    addoption(_("End marker expansion/contraction (s)."), mMarkerEndAddition);

    addbox(_("Clips"));
    mStrip = new wxTextCtrl(mPanel, wxID_ANY, wxConfigBase::Get()->Read(Config::sPathStrip,""));
    addoption(_("Text to remove once from clip names (requires restart)"), mStrip);

    //////////////////////////////////////////////////////////////////////////

    addtab(_("Debug"));

    addbox(_("Logging"));

    mLogLevel = new wxChoice(mPanel, wxID_ANY);
    mLogLevel->Append(_("Error"),      reinterpret_cast<void*>(logERROR));
    mLogLevel->Append(_("Warning"),    reinterpret_cast<void*>(logWARNING));
    mLogLevel->Append(_("Info"),       reinterpret_cast<void*>(logINFO));
    mLogLevel->Append(_("Debug"),      reinterpret_cast<void*>(logDEBUG));
    mLogLevel->Append(_("Video"),      reinterpret_cast<void*>(logVIDEO));
    mLogLevel->Append(_("Audio"),      reinterpret_cast<void*>(logAUDIO));
    mLogLevel->Append(_("Detailed"),   reinterpret_cast<void*>(logDETAIL));
    switch (LogLevel_fromString(std::string(wxConfigBase::Get()->Read(Config::sPathLogLevel,"logINFO").mb_str())))
    {
    case logERROR:      mLogLevel->SetSelection(0); break;
    case logWARNING:    mLogLevel->SetSelection(1); break;
    case logINFO:       mLogLevel->SetSelection(2); break;
    case logDEBUG:      mLogLevel->SetSelection(3); break;
    case logVIDEO:      mLogLevel->SetSelection(4); break;
    case logAUDIO:      mLogLevel->SetSelection(5); break;
    case logDETAIL:     mLogLevel->SetSelection(6); break;
    }
    addoption(_("Log level"), mLogLevel);

    mShowDebugInfoOnWidgets = new wxCheckBox(mPanel, wxID_ANY, _T(""));
    mShowDebugInfoOnWidgets->SetValue(wxConfigBase::Get()->ReadBool(Config::sPathShowDebugInfoOnWidgets,false));
    addoption(_("Show debug info on widgets (requires restart)"), mShowDebugInfoOnWidgets);

    //////////////////////////////////////////////////////////////////////////

    SetExtraStyle(wxDIALOG_EX_CONTEXTHELP|wxWS_EX_VALIDATE_RECURSIVELY);
    CreateButtons(wxOK | wxCANCEL);

    LayoutDialog();
}

Options::~Options()
{
    if (GetReturnCode() == GetAffirmativeId())
    {
        wxConfigBase::Get()->Write( Config::sPathAutoLoadEnabled,           mLoadLast->IsChecked());
        wxConfigBase::Get()->Write( Config::sPathLogLevel,                  LogLevel_toString(static_cast<LogLevel>(reinterpret_cast<int>(mLogLevel->GetClientData(mLogLevel->GetSelection())))).c_str());
        wxConfigBase::Get()->Write( Config::sPathShowDebugInfoOnWidgets,    mShowDebugInfoOnWidgets->IsChecked());
        wxConfigBase::Get()->Write( Config::sPathFrameRate,                 framerate::toString(framerate::getSupported()[mFrameRate->GetSelection()]));
        wxConfigBase::Get()->Write( Config::sPathMarkerBeginAddition,       mMarkerBeginAddition->GetValue());
        wxConfigBase::Get()->Write( Config::sPathMarkerEndAddition,         mMarkerEndAddition->GetValue());
        wxConfigBase::Get()->Write( Config::sPathStrip,                     mStrip->GetValue());

        wxConfigBase::Get()->Flush();

        // Use new values
        Log::SetReportingLevel(LogLevel_fromString(std::string(wxConfigBase::Get()->Read(Config::sPathLogLevel,"logINFO").mb_str())));
    }
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Options::addtab(const wxString& name)
{
    mPanel = new wxPanel(GetBookCtrl(), wxID_ANY);
    GetBookCtrl()->AddPage(mPanel, name, true);
    mTopSizer = new wxBoxSizer( wxVERTICAL );
    mPanel->SetSizerAndFit(mTopSizer);
    mBoxSizer = 0;
}

void Options::addbox(const wxString& name)
{
    ASSERT(mPanel);
    ASSERT(mTopSizer);
    mBoxSizer = new wxStaticBoxSizer(new wxStaticBox(mPanel, wxID_ANY, name), wxVERTICAL );
    mTopSizer->Add(mBoxSizer, 1, wxGROW|wxALIGN_CENTRE|wxALL, 5 );
}

void Options::addoption(const wxString& name, wxWindow* widget)
{
    ASSERT(mBoxSizer);
    wxBoxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );
    mBoxSizer->Add(hSizer, 0, wxGROW|wxLEFT|wxALL, 5);
    hSizer->Add(new wxStaticText(mPanel, wxID_ANY, name), 0, wxALL|wxALIGN_TOP, 5);
    hSizer->Add(5, 5, 1, wxALL, 0);
    hSizer->Add(widget, 0, wxRIGHT|wxALIGN_TOP, 5);
}

} // namespace
