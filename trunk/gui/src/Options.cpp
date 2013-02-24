#include "Options.h"

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
        BOOST_FOREACH( FrameRate fr, FrameRate::getSupported() )
        {
            wxString frs = fr.toString();
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

        mDefaultVideoScaling = new EnumSelector<model::VideoScaling>(mPanel, model::VideoScalingConverter::mapToHumanReadibleString, model::VideoScalingConverter::readConfigValue(Config::sPathDefaultVideoScaling));
        addoption(_("Default video scaling"), mDefaultVideoScaling);

        mDefaultVideoAlignment = new EnumSelector<model::VideoAlignment>(mPanel, model::VideoAlignmentConverter::mapToHumanReadibleString, model::VideoAlignmentConverter::readConfigValue(Config::sPathDefaultVideoAlignment));
        addoption(_("Default video scaling"), mDefaultVideoAlignment);
    }
    {
        addtab(_("Audio"));

        addbox(_("New projects"));

         wxArrayString sampleRateChoices;
         sampleRateChoices.Add("22050");
         sampleRateChoices.Add("44100");
         sampleRateChoices.Add("48000");
         wxIntegerValidator<int> sampleRateValidator;
         sampleRateValidator.SetMin(1000);
         sampleRateValidator.SetMax(1000);
         long initial = Config::ReadLong(Config::sPathDefaultAudioSampleRate);
         mDefaultAudioSampleRate = new wxComboBox(mPanel, wxID_ANY, wxString::Format("%d", initial),  wxDefaultPosition, wxDefaultSize, sampleRateChoices, 0, sampleRateValidator);
         addoption(_("Default audio sample rate"), mDefaultAudioSampleRate);

         wxIntegerValidator<int> channelValidator;
         channelValidator.SetMin(1);
         channelValidator.SetMax(2);
         wxArrayString channelChoices;
         channelChoices.Add("1");
         channelChoices.Add("2");
         initial = Config::ReadLong(Config::sPathDefaultAudioChannels);
         mDefaultAudioNumberOfChannels = new wxComboBox(mPanel, wxID_ANY, wxString::Format("%d", initial),  wxDefaultPosition, wxDefaultSize, channelChoices, 0, channelValidator);
         addoption(_("Default number of audio channels"), mDefaultAudioNumberOfChannels);
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
        addoption(_("Text to remove from clip names - use '|' for multiple entries \r\n(requires restart)"), mStrip);
    }
    {
        addtab(_("Debug"));

        addbox(_("Logging"));

        mSelectLogLevel = new EnumSelector<LogLevel>(mPanel, LogLevelConverter::mapToHumanReadibleString, LogLevelConverter::readConfigValue(Config::sPathLogLevel));
        addoption(_("Log level"), mSelectLogLevel);

        mSelectLogLevelAvcodec = new EnumSelector<int>(mPanel, Avcodec::mapAvcodecLevels, Avcodec::mapAvcodecLevels.right.at(Config::ReadString(Config::sPathLogLevelAvcodec)));
        addoption(_("Avcodec log level (requires restart)"), mSelectLogLevelAvcodec);

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
        long value(0);
        bool ok(false);
        Config::holdWriteToDisk();
        Config::WriteBool( Config::sPathAutoLoadEnabled,           mLoadLast->IsChecked());
        Config::WriteString( Config::sPathLogLevel,                  LogLevel_toString(mSelectLogLevel->getValue()).c_str());
        Config::WriteString( Config::sPathLogLevelAvcodec,           Avcodec::mapAvcodecLevels.left.at(mSelectLogLevelAvcodec->getValue()));
        Config::WriteBool( Config::sPathShowDebugInfoOnWidgets,    mShowDebugInfoOnWidgets->IsChecked());
        Config::WriteString( Config::sPathDefaultFrameRate,          (FrameRate::getSupported()[mFrameRate->GetSelection()]).toString());
        Config::WriteLong( Config::sPathDefaultVideoWidth,         mDefaultVideoWidth->GetValue());
        Config::WriteLong( Config::sPathDefaultVideoHeight,        mDefaultVideoHeight->GetValue());
        Config::WriteString( Config::sPathDefaultVideoScaling,       model::VideoScaling_toString(mDefaultVideoScaling->getValue()).c_str());
        Config::WriteString( Config::sPathDefaultVideoAlignment,     model::VideoAlignment_toString(mDefaultVideoAlignment->getValue()).c_str());
        ok = mDefaultAudioSampleRate->GetValue().ToLong(&value); ASSERT(ok);
        Config::WriteLong( Config::sPathDefaultAudioSampleRate, value);
        ok = mDefaultAudioNumberOfChannels->GetValue().ToLong(&value); ASSERT(ok);
        Config::WriteLong( Config::sPathDefaultAudioChannels, value);
        Config::WriteLong( Config::sPathMarkerBeginAddition,       mMarkerBeginAddition->GetValue());
        Config::WriteLong( Config::sPathMarkerEndAddition,         mMarkerEndAddition->GetValue());
        Config::WriteString( Config::sPathStrip,                     mStrip->GetValue());
        Config::releaseWriteToDisk();

        // Use new values
        Log::setReportingLevel(LogLevelConverter::readConfigValue(Config::sPathLogLevel));
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