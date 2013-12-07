// Copyright 2013 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#include "DialogOptions.h"

#include "Config.h"
#include "Constants.h"
#include "Convert.h"
#include "Enums.h"
#include "Properties.h"
#include "UtilFrameRate.h"
#include "UtilInitAvcodec.h"
#include "UtilLog.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

DialogOptions::DialogOptions(wxWindow* win)
    :   wxPropertySheetDialog(win, wxID_ANY, _("Preferences"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
    ,   mPanel(0)
    ,   mTopSizer(0)
    ,   mBoxSizer(0)
{
    wxIconBundle icons;
    icons.AddIcon(Config::getExeDir() + "\\icons\\movie_all.ico"); // Icon in title bar of window
    SetIcons(icons);

    {
        addtab(_("Load/Save"));

        addbox(_("Startup"));

        mLoadLast = new wxCheckBox(mPanel, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize);
        mLoadLast->SetValue(Config::ReadBool(Config::sPathAutoLoadEnabled));
        addoption(_("Load last project on startup"), mLoadLast);

        addbox(_("Save"));

        mBackupBeforeSave = new wxCheckBox(mPanel, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize);
        mBackupBeforeSave->SetValue(Config::ReadBool(Config::sPathBackupBeforeSaveEnabled));
        addoption(_("Make backup of existing save file when overwriting"), mBackupBeforeSave);

        long maximum = Config::ReadLong(Config::sPathBackupBeforeSaveMaximum);
        mBackupBeforeSaveMaximum = new wxSpinCtrl(mPanel, wxID_ANY, wxString::Format("%d", maximum), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, 0, 10000, maximum);
        addoption(_("Maximum number of generated save files (0 - infinite)"), mBackupBeforeSaveMaximum);
    }
    {
        addtab(_("Project view"));

        addbox(_("Make sequence: divide clips if clip's prefix differs"));

        long initial = Config::ReadLong(Config::sPathMakeSequenceEmptyClipLength);
        mMakeSequenceEmptyLength = new wxSpinCtrl(mPanel, wxID_ANY, wxString::Format("%d", initial), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, 0, 100000, initial);
        addoption(_("Length of empty division (0 - disabled)"), mMakeSequenceEmptyLength);

        initial = Config::ReadLong(Config::sPathMakeSequencePrefixLength);
        mMakeSequencePrefixLength = new wxSpinCtrl(mPanel, wxID_ANY, wxString::Format("%d", initial), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, 0, 100000, initial);
        addoption(_("Length of name (prefix) to be matched"), mMakeSequencePrefixLength);
    }
    {
        addtab(_("Video"));

        addbox(_("New projects"));

        wxArrayString choices;
        unsigned int selection = 0;
        wxString currentFrameRate = Config::ReadString(Config::sPathDefaultFrameRate);
        for ( FrameRate fr : FrameRate::getSupported() )
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
        addoption(_("Default video alignment"), mDefaultVideoAlignment);
    }
    {
        addtab(_("Images"));

        addbox(_("Size"));

         wxIntegerValidator<int> lengthValidator;
         lengthValidator.SetMin(1);
         lengthValidator.SetMax(10000);
         pts initial = Config::ReadLong(Config::sPathDefaultStillImageLength);

         FrameRate framerate = FrameRate::s25p; // Default
         if (Window::get().GetDocumentManager()->GetCurrentDocument() != 0)
         {
             // Only if a project is opened use that project's frame rate
             framerate = model::Properties::get().getFrameRate();
         }
         std::list<pts> values = boost::assign::list_of
             (1)
             (12)
             (1  * model::Convert::timeToPts(model::Constants::sSecond, framerate))
             (2  * model::Convert::timeToPts(model::Constants::sSecond, framerate))
             (3  * model::Convert::timeToPts(model::Constants::sSecond, framerate))
             (4  * model::Convert::timeToPts(model::Constants::sSecond, framerate))
             (5  * model::Convert::timeToPts(model::Constants::sSecond, framerate))
             (6  * model::Convert::timeToPts(model::Constants::sSecond, framerate))
             (10 * model::Convert::timeToPts(model::Constants::sSecond, framerate))
             (1  * model::Convert::timeToPts(model::Constants::sMinute, framerate))
             (initial);
         values.sort();
         values.unique();

         wxArrayString choices;
         for ( pts value : values )
         {
             choices.Add(wxString::Format("%" PRId64, value ));
         }
         mDefaultStillImageLength = new wxComboBox(mPanel, wxID_ANY, wxString::Format("%" PRId64, initial),  wxDefaultPosition, wxDefaultSize, choices, 0, lengthValidator);
         addoption(_("Default length of still images (in frames)"), mDefaultStillImageLength);
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

        mLogSequenceOnEdit = new wxCheckBox(mPanel, wxID_ANY, _T(""));
        mLogSequenceOnEdit->SetValue(Config::ReadBool(Config::sPathDebugLogSequenceOnEdit));
        addoption(_("Log the current sequence after each edit operation"), mLogSequenceOnEdit);
    }

    SetExtraStyle(wxDIALOG_EX_CONTEXTHELP|wxWS_EX_VALIDATE_RECURSIVELY);
    CreateButtons(wxOK | wxCANCEL);

    LayoutDialog();

}

DialogOptions::~DialogOptions()
{
    auto toLong = [](wxString comboBoxValue) -> long
    {
        long value(0);
        bool ok(false);
        ok = comboBoxValue.ToLong(&value);
        ASSERT(ok);
        return value;
    };

    if (GetReturnCode() == GetAffirmativeId())
    {
        Config::holdWriteToDisk();
        Config::WriteBool(      Config::sPathAutoLoadEnabled,             mLoadLast->IsChecked());
        Config::WriteBool(      Config::sPathBackupBeforeSaveEnabled,     mBackupBeforeSave->IsChecked());
        Config::WriteLong(      Config::sPathBackupBeforeSaveMaximum,     mBackupBeforeSaveMaximum->GetValue());
        Config::WriteLong(      Config::sPathMakeSequenceEmptyClipLength, mMakeSequenceEmptyLength->GetValue());
        Config::WriteLong(      Config::sPathMakeSequencePrefixLength,    mMakeSequencePrefixLength->GetValue());
        Config::WriteString(    Config::sPathLogLevel,                    LogLevel_toString(mSelectLogLevel->getValue()).c_str());
        Config::WriteString(    Config::sPathLogLevelAvcodec,             Avcodec::mapAvcodecLevels.left.at(mSelectLogLevelAvcodec->getValue()));
        Config::WriteBool(      Config::sPathShowDebugInfoOnWidgets,      mShowDebugInfoOnWidgets->IsChecked());
        Config::WriteBool(      Config::sPathDebugLogSequenceOnEdit,      mLogSequenceOnEdit->IsChecked());
        Config::WriteString(    Config::sPathDefaultFrameRate,            (FrameRate::getSupported()[mFrameRate->GetSelection()]).toString());
        Config::WriteLong(      Config::sPathDefaultVideoWidth,           mDefaultVideoWidth->GetValue());
        Config::WriteLong(      Config::sPathDefaultVideoHeight,          mDefaultVideoHeight->GetValue());
        Config::WriteString(    Config::sPathDefaultVideoScaling,         model::VideoScaling_toString(mDefaultVideoScaling->getValue()).c_str());
        Config::WriteString(    Config::sPathDefaultVideoAlignment,       model::VideoAlignment_toString(mDefaultVideoAlignment->getValue()).c_str());
        Config::WriteLong(      Config::sPathDefaultAudioSampleRate,      toLong(mDefaultAudioSampleRate->GetValue()));
        Config::WriteLong(      Config::sPathDefaultAudioChannels,        toLong(mDefaultAudioNumberOfChannels->GetValue()));
        Config::WriteLong(      Config::sPathMarkerBeginAddition,         mMarkerBeginAddition->GetValue());
        Config::WriteLong(      Config::sPathMarkerEndAddition,           mMarkerEndAddition->GetValue());
        Config::WriteLong(      Config::sPathDefaultStillImageLength,     toLong(mDefaultStillImageLength->GetValue()));
        Config::WriteString(    Config::sPathStrip,                       mStrip->GetValue());
        Config::releaseWriteToDisk();

        // Use new values
        Log::setReportingLevel(LogLevelConverter::readConfigValue(Config::sPathLogLevel));
    }
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void DialogOptions::addtab(const wxString& name)
{
    mPanel = new wxPanel(GetBookCtrl(), wxID_ANY);
    GetBookCtrl()->AddPage(mPanel, name, true);
    mTopSizer = new wxBoxSizer( wxVERTICAL );
    mPanel->SetSizerAndFit(mTopSizer);
    mBoxSizer = 0;
}

void DialogOptions::addbox(const wxString& name)
{
    ASSERT(mPanel);
    ASSERT(mTopSizer);
    mBoxSizer = new wxStaticBoxSizer(new wxStaticBox(mPanel, wxID_ANY, name), wxVERTICAL );
    mTopSizer->Add(mBoxSizer, 1, wxGROW|wxALIGN_CENTRE|wxALL, 5 );
}

void DialogOptions::addoption(const wxString& name, wxWindow* widget)
{
    ASSERT(mBoxSizer);
    wxBoxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );
    mBoxSizer->Add(hSizer, 0, wxGROW|wxLEFT|wxALL, 5);
    hSizer->Add(new wxStaticText(mPanel, wxID_ANY, name), 0, wxALL|wxALIGN_TOP, 5);
    hSizer->Add(5, 5, 1, wxALL, 0);
    hSizer->Add(widget, 0, wxRIGHT|wxALIGN_TOP, 5);
}

} // namespace