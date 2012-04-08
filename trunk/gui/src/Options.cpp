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
#include "Enums.h"
#include "UtilLog.h"
#include "UtilInitAvcodec.h"
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
    {
        addtab(_("General"));

        addbox(_("Startup"));

        mLoadLast = new wxCheckBox(mPanel, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize);
        mLoadLast->SetValue(Config::ReadBool(Config::sPathAutoLoadEnabled));
        addoption(_("Load last project on startup"), mLoadLast);
    }
    {
        addtab(_("Video"));

        addbox(_("New projects"));

        wxArrayString choices;
        unsigned int selection = 0;
        wxString currentFrameRate = Config::ReadString(Config::sPathDefaultFrameRate);
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

        long initial = Config::ReadLong(Config::sPathDefaultVideoWidth);
        mDefaultVideoWidth = new wxSpinCtrl(mPanel, wxID_ANY, wxString::Format("%d", initial), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, 20, 10000, initial);
        addoption(_("Default video width"), mDefaultVideoWidth);

        initial = Config::ReadLong(Config::sPathDefaultVideoHeight);
        mDefaultVideoHeight = new wxSpinCtrl(mPanel, wxID_ANY, wxString::Format("%d", initial), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, 20, 10000, initial);
        addoption(_("Default video height"), mDefaultVideoHeight);

        mDefaultVideoScaling = new wxChoice(mPanel, wxID_ANY);
        mDefaultVideoScaling->Append(_("Fit all"),       reinterpret_cast<void*>(model::VideoScalingFitAll));
        mDefaultVideoScaling->Append(_("Fit to fill"),   reinterpret_cast<void*>(model::VideoScalingFitToFill));
        mDefaultVideoScaling->Append(_("Original size"), reinterpret_cast<void*>(model::VideoScalingNone));
        mDefaultVideoScaling->Append(_("Custom"),        reinterpret_cast<void*>(model::VideoScalingCustom));
        switch (model::VideoScaling_fromString(std::string(Config::ReadString(Config::sPathDefaultVideoScaling).mb_str())))
        {
        case model::VideoScalingFitAll:       mDefaultVideoScaling->SetSelection(0); break;
        case model::VideoScalingFitToFill:    mDefaultVideoScaling->SetSelection(1); break;
        case model::VideoScalingNone:         mDefaultVideoScaling->SetSelection(2); break;
        case model::VideoScalingCustom:       mDefaultVideoScaling->SetSelection(3); break;
        }
        addoption(_("Default video scaling"), mDefaultVideoScaling);

        mDefaultVideoAlignment = new wxChoice(mPanel, wxID_ANY);
        mDefaultVideoAlignment->Append(_("Centered"), reinterpret_cast<void*>(model::VideoAlignmentCenter));
        mDefaultVideoAlignment->Append(_("Custom"),   reinterpret_cast<void*>(model::VideoAlignmentCustom));
        switch (model::VideoAlignment_fromString(std::string(Config::ReadString(Config::sPathDefaultVideoAlignment).mb_str())))
        {
        case model::VideoAlignmentCenter:    mDefaultVideoAlignment->SetSelection(0); break;
        case model::VideoAlignmentCustom:    mDefaultVideoAlignment->SetSelection(1); break;
        }
        addoption(_("Default video scaling"), mDefaultVideoAlignment);
    }
    {
        addtab(_("Timeline"));

        addbox(_("Marking selection"));

        double initial = Config::ReadDouble(Config::sPathMarkerBeginAddition);
        mMarkerBeginAddition = new wxSpinCtrlDouble(mPanel, wxID_ANY, wxString::Format("%1.1f", initial), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, -10, 10, initial, 0.1);
        addoption(_("Begin marker expansion/contraction (s)."), mMarkerBeginAddition);

        initial = Config::ReadDouble(Config::sPathMarkerEndAddition);
        mMarkerEndAddition = new wxSpinCtrlDouble(mPanel, wxID_ANY, wxString::Format("%1.1f", initial), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, -10, 10, initial, 0.1);
        addoption(_("End marker expansion/contraction (s)."), mMarkerEndAddition);

        addbox(_("Clips"));
        mStrip = new wxTextCtrl(mPanel, wxID_ANY, Config::ReadString(Config::sPathStrip));
        addoption(_("Text to remove once from clip names (requires restart)"), mStrip);
    }
    {
        addtab(_("Debug"));

        addbox(_("Logging"));

        mLogLevel = new wxChoice(mPanel, wxID_ANY);
        mLogLevel->Append(_("Error"),      reinterpret_cast<void*>(LogError));
        mLogLevel->Append(_("Warning"),    reinterpret_cast<void*>(LogWarning));
        mLogLevel->Append(_("Info"),       reinterpret_cast<void*>(LogInfo));
        mLogLevel->Append(_("Debug"),      reinterpret_cast<void*>(LogDebug));
        mLogLevel->Append(_("Video"),      reinterpret_cast<void*>(LogVideo));
        mLogLevel->Append(_("Audio"),      reinterpret_cast<void*>(LogAudio));
        mLogLevel->Append(_("Detailed"),   reinterpret_cast<void*>(LogDetail));
        switch (LogLevel_fromString(std::string(Config::ReadString(Config::sPathLogLevel).mb_str())))
        {
        case LogError:      mLogLevel->SetSelection(0); break;
        case LogWarning:    mLogLevel->SetSelection(1); break;
        case LogInfo:       mLogLevel->SetSelection(2); break;
        case LogDebug:      mLogLevel->SetSelection(3); break;
        case LogVideo:      mLogLevel->SetSelection(4); break;
        case LogAudio:      mLogLevel->SetSelection(5); break;
        case LogDetail:     mLogLevel->SetSelection(6); break;
        }
        addoption(_("Log level"), mLogLevel);

        mLogLevelAvcodec = new wxChoice(mPanel, wxID_ANY);
        int current = 0;
        bool currentfound = false;
        BOOST_FOREACH( auto value, Avcodec::getLogLevels() )
        {
            mLogLevelAvcodec->Append(value);
            if (!currentfound && !Config::ReadString(Config::sPathLogLevelAvcodec).IsSameAs(value))
            {
                current++;
            }
            else
            {
                currentfound = true;
            }
        }
        mLogLevelAvcodec->SetSelection(currentfound ? current : 0);
        addoption(_("Avcodec log level (requires restart)"), mLogLevelAvcodec);

        mShowDebugInfoOnWidgets = new wxCheckBox(mPanel, wxID_ANY, _T(""));
        mShowDebugInfoOnWidgets->SetValue(Config::ReadBool(Config::sPathShowDebugInfoOnWidgets)); // Do not read cached value, but the last set value
        addoption(_("Show debug info on widgets (requires restart)"), mShowDebugInfoOnWidgets);
    }

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
        wxConfigBase::Get()->Write( Config::sPathLogLevelAvcodec,           mLogLevelAvcodec->GetString(mLogLevelAvcodec->GetSelection()));
        wxConfigBase::Get()->Write( Config::sPathShowDebugInfoOnWidgets,    mShowDebugInfoOnWidgets->IsChecked());
        wxConfigBase::Get()->Write( Config::sPathDefaultFrameRate,          framerate::toString(framerate::getSupported()[mFrameRate->GetSelection()]));
        wxConfigBase::Get()->Write( Config::sPathDefaultVideoWidth,         mDefaultVideoWidth->GetValue());
        wxConfigBase::Get()->Write( Config::sPathDefaultVideoHeight,        mDefaultVideoHeight->GetValue());
        wxConfigBase::Get()->Write( Config::sPathDefaultVideoScaling,       model::VideoScaling_toString(static_cast<model::VideoScaling>(reinterpret_cast<int>(mDefaultVideoScaling->GetClientData(mDefaultVideoScaling->GetSelection())))).c_str());
        wxConfigBase::Get()->Write( Config::sPathDefaultVideoAlignment,     model::VideoAlignment_toString(static_cast<model::VideoAlignment>(reinterpret_cast<int>(mDefaultVideoAlignment->GetClientData(mDefaultVideoAlignment->GetSelection())))).c_str());
        wxConfigBase::Get()->Write( Config::sPathMarkerBeginAddition,       mMarkerBeginAddition->GetValue());
        wxConfigBase::Get()->Write( Config::sPathMarkerEndAddition,         mMarkerEndAddition->GetValue());
        wxConfigBase::Get()->Write( Config::sPathStrip,                     mStrip->GetValue());

        wxConfigBase::Get()->Flush();

        // Use new values
        Log::setReportingLevel(LogLevel_fromString(std::string(Config::ReadString(Config::sPathLogLevel).mb_str())));
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