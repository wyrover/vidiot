// Copyright 2013-2015 Eric Raijmakers.
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
#include "Convert.h"
#include "Enums.h"
#include "Properties.h"
#include "UtilFrameRate.h"
#include "UtilInitAvcodec.h"
#include "UtilLocale.h"
#include "UtilLog.h"
#include "UtilMap.h"
#include "UtilWindow.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

DialogOptions::DialogOptions(wxWindow* win)
    : wxPropertySheetDialog()
    , mPanel(0)
    , mTopSizer(0)
    , mBoxSizer(0)
    , mIcons(16,16)
{
    util::window::setIcons(this);

    wxStrings icons = {
        { "disks.png" },
        { "folder-horizontal-open.png" },
        { "clapperboard.png" },
        { "picture.png" },
        { "music-beam.png" },
        { "film.png" },
        { "application.png" },
        { "bug.png" },
    };
    for (wxString icon : icons)
    {
        mIcons.Add(util::window::getIcon(icon));
    }

    wxString sRestart{ " " + _("(requires restart)") };

    Create(win, wxID_ANY, _("Options"), wxDefaultPosition, wxSize(1000, -1), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    GetBookCtrl()->SetImageList(&mIcons);
    {
        addtab(_("Load/Save"));

        addbox(_("Startup"));

        mLoadLast = new wxCheckBox(mPanel, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize);
        mLoadLast->SetValue(Config::ReadBool(Config::sPathProjectAutoLoadEnabled));
        addoption(_("Load last project on startup"), mLoadLast);

        addbox(_("Save"));

        mBackupBeforeSave = new wxCheckBox(mPanel, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize);
        mBackupBeforeSave->SetValue(Config::ReadBool(Config::sPathProjectBackupBeforeSaveEnabled));
        addoption(_("Make backup of existing save file when overwriting"), mBackupBeforeSave);

        long maximum = Config::ReadLong(Config::sPathProjectBackupBeforeSaveMaximum);
        mBackupBeforeSaveMaximum = new wxSpinCtrl(mPanel, wxID_ANY, wxString::Format("%ld", maximum), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, 0, 10000, maximum);
        addoption(_("Maximum number of generated save files (0 - infinite)"), mBackupBeforeSaveMaximum);

        mSaveAbsolute = new wxCheckBox(mPanel, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize);
        mSaveAbsolute->SetValue(Config::ReadBool(Config::sPathProjectSavePathsRelativeToProject));
        addoption(_("Use absolute path to media files when saving projects"), mSaveAbsolute);
        addnote(_("By using absolute paths, the project file can be moved without moving any media files in the project. Otherwise, project and media files must be moved together."));
    }
    {
        addtab(_("Project view"));

        addbox(_("Make sequence: divide clips if clip's prefix differs"));

        long initial = Config::ReadLong(Config::sPathMakeSequenceEmptyClipLength);
        mMakeSequenceEmptyLength = new wxSpinCtrl(mPanel, wxID_ANY, wxString::Format("%ld", initial), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, 0, 100000, initial);
        addoption(_("Length of empty division (0 - disabled)"), mMakeSequenceEmptyLength);

        initial = Config::ReadLong(Config::sPathMakeSequencePrefixLength);
        mMakeSequencePrefixLength = new wxSpinCtrl(mPanel, wxID_ANY, wxString::Format("%ld", initial), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, 0, 100000, initial);
        addoption(_("Length of name (prefix) to be matched"), mMakeSequencePrefixLength);
    }
    {
        addtab(_("Video"));

        addbox(_("New projects"));

        wxArrayString choices;
        unsigned int selection = 0;
        wxString currentFrameRate = Config::ReadString(Config::sPathVideoDefaultFrameRate);
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

        long initial = Config::ReadLong(Config::sPathVideoDefaultWidth);
        mDefaultVideoWidth = new wxSpinCtrl(mPanel, wxID_ANY, wxString::Format("%ld", initial), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, 20, 10000, initial);
        addoption(_("Default video width"), mDefaultVideoWidth);

        initial = Config::ReadLong(Config::sPathVideoDefaultHeight);
        mDefaultVideoHeight = new wxSpinCtrl(mPanel, wxID_ANY, wxString::Format("%ld", initial), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, 20, 10000, initial);
        addoption(_("Default video height"), mDefaultVideoHeight);

        mDefaultVideoScaling = new EnumSelector<model::VideoScaling>(mPanel, model::VideoScalingConverter::getMapToHumanReadibleString(), model::VideoScalingConverter::readConfigValue(Config::sPathVideoDefaultScaling));
        addoption(_("Default video scaling"), mDefaultVideoScaling);

        mDefaultVideoAlignment = new EnumSelector<model::VideoAlignment>(mPanel, model::VideoAlignmentConverter::getMapToHumanReadibleString(), model::VideoAlignmentConverter::readConfigValue(Config::sPathVideoDefaultAlignment));
        addoption(_("Default video alignment"), mDefaultVideoAlignment);
    }
    {
        addtab(_("Images"));

        addbox(_("Size"));

         wxIntegerValidator<int> lengthValidator;
         lengthValidator.SetMin(1);
         lengthValidator.SetMax(10000);
         pts initial = Config::ReadLong(Config::sPathTimelineDefaultStillImageLength);

         FrameRate framerate = FrameRate::s25p; // Default
         if (Window::get().GetDocumentManager()->GetCurrentDocument() != 0)
         {
             // Only if a project is opened use that project's frame rate
             framerate = model::Properties::get().getFrameRate();
         }
         std::list<pts> values {
             { 1 },
             { 12 },
             { 1 * model::Convert::timeToPts(sSecond, framerate) },
             { 2 * model::Convert::timeToPts(sSecond, framerate) },
             { 3 * model::Convert::timeToPts(sSecond, framerate) },
             { 4 * model::Convert::timeToPts(sSecond, framerate) },
             { 5 * model::Convert::timeToPts(sSecond, framerate) },
             { 6 * model::Convert::timeToPts(sSecond, framerate) },
             { 10 * model::Convert::timeToPts(sSecond, framerate) },
             { 1 * model::Convert::timeToPts(sMinute, framerate) },
             { initial },
         };
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
         long initial = Config::ReadLong(Config::sPathAudioDefaultSampleRate);
         mDefaultAudioSampleRate = new wxComboBox(mPanel, wxID_ANY, wxString::Format("%ld", initial),  wxDefaultPosition, wxDefaultSize, sampleRateChoices, 0, sampleRateValidator);
         addoption(_("Default audio sample rate"), mDefaultAudioSampleRate);

         wxIntegerValidator<int> channelValidator;
         channelValidator.SetMin(1);
         channelValidator.SetMax(2);
         wxArrayString channelChoices;
         channelChoices.Add("1");
         channelChoices.Add("2");
         initial = Config::ReadLong(Config::sPathAudioDefaultNumberOfChannels);
         mDefaultAudioNumberOfChannels = new wxComboBox(mPanel, wxID_ANY, wxString::Format("%ld", initial),  wxDefaultPosition, wxDefaultSize, channelChoices, 0, channelValidator);
         addoption(_("Default number of audio channels"), mDefaultAudioNumberOfChannels);
    }
    {
        addtab(_("Timeline"));

        addbox(_("Marking selection"));

        double initial = Config::ReadDouble(Config::sPathTimelineMarkerBeginAddition);
        mMarkerBeginAddition = new wxSpinCtrlDouble(mPanel, wxID_ANY, wxString::Format("%1.1f", initial), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, -10, 10, initial, 0.1);
        addoption(_("Begin marker expansion/contraction (s)."), mMarkerBeginAddition);

        initial = Config::ReadDouble(Config::sPathTimelineMarkerEndAddition);
        mMarkerEndAddition = new wxSpinCtrlDouble(mPanel, wxID_ANY, wxString::Format("%1.1f", initial), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, -10, 10, initial, 0.1);
        addoption(_("End marker expansion/contraction (s)."), mMarkerEndAddition);

        addbox(_("Clips"));
        mStrip = new wxTextCtrl(mPanel, wxID_ANY, Config::ReadString(Config::sPathTimelineStripFromClipNames));
        addoption(_("Text to remove from clip names - use '|' for multiple entries") + "\n" + sRestart, mStrip);

        addbox(_("Behaviour"));

        mTimelineEnableAutoAddTracks = new wxCheckBox(mPanel, wxID_ANY, _T(""), wxDefaultPosition, wxDefaultSize);
        mTimelineEnableAutoAddTracks->SetValue(Config::ReadBool(Config::sPathTimelineAutoAddEmptyTrackWhenDragging));
        addoption(_("Automatically add audio/video track when dragging beyond existing tracks"), mTimelineEnableAutoAddTracks);
    }
    {
        addtab(_("Application"));

        addbox(_("Language"));

        mLanguage = new wxListBox (mPanel, wxID_ANY, wxDefaultPosition, wxSize(200, -1), 0, nullptr, wxLB_SINGLE | wxLB_NEEDED_SB | wxLB_SORT, wxDefaultValidator, wxListBoxNameStr);

        wxString currentLanguage{ Config::ReadString(Config::sPathWorkspaceLanguage) };

        for (auto lang : getSupportedLanguages())
        {
            mLanguage->Append(lang.first);
            if (currentLanguage == lang.second)
            {
                mLanguage->SetStringSelection(lang.first);
            }
        }

        addoption(_("Language") + sRestart, mLanguage);
    }
    {
        addtab(_("Debug"));

        addbox(_("Logging"));

        mSelectLogLevel = new EnumSelector<LogLevel>(mPanel, LogLevelConverter::getMapToHumanReadibleString(), LogLevelConverter::readConfigValue(Config::sPathDebugLogLevel));
        addoption(_("Log level"), mSelectLogLevel);

        mSelectLogLevelAvcodec = new EnumSelector<LogLevelAvcodec>(mPanel, LogLevelAvcodecConverter::getMapToHumanReadibleString(), LogLevelAvcodecConverter::readConfigValue(Config::sPathDebugLogLevelAvcodec));
        addoption(_("Avcodec log level") + sRestart, mSelectLogLevelAvcodec);

        mShowDebugInfoOnWidgets = new wxCheckBox(mPanel, wxID_ANY, _T(""));
        mShowDebugInfoOnWidgets->SetValue(Config::ReadBool(Config::sPathDebugShowDebugInfoOnWidgets)); // Do not read cached value, but the last set value
        addoption(_("Show debug info on widgets") + sRestart, mShowDebugInfoOnWidgets);

        mLogSequenceOnEdit = new wxCheckBox(mPanel, wxID_ANY, _T(""));
        mLogSequenceOnEdit->SetValue(Config::ReadBool(Config::sPathDebugLogSequenceOnEdit));
        addoption(_("Log the current sequence after each edit operation"), mLogSequenceOnEdit);
    }

    SetExtraStyle(wxDIALOG_EX_CONTEXTHELP|wxWS_EX_VALIDATE_RECURSIVELY);
    CreateButtons(wxOK | wxCANCEL);

    ASSERT_EQUALS(icons.size(), GetBookCtrl()->GetPageCount()); // Ensure the proper amount of icons
    LayoutDialog();
    SetSize(wxSize(700, -1));

    gui::Window::get().setDialogOpen(true);
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
        Config::WriteBool(Config::sPathProjectAutoLoadEnabled, mLoadLast->IsChecked());
        Config::WriteBool(Config::sPathProjectBackupBeforeSaveEnabled, mBackupBeforeSave->IsChecked());
        Config::WriteLong(Config::sPathProjectBackupBeforeSaveMaximum, mBackupBeforeSaveMaximum->GetValue());
        Config::WriteBool(Config::sPathProjectSavePathsRelativeToProject, mSaveAbsolute->IsChecked());
        Config::WriteLong(Config::sPathMakeSequenceEmptyClipLength, mMakeSequenceEmptyLength->GetValue());
        Config::WriteLong(Config::sPathMakeSequencePrefixLength, mMakeSequencePrefixLength->GetValue());
        Config::WriteString(Config::sPathDebugLogLevel, LogLevel_toString(mSelectLogLevel->getValue()).c_str());
        Config::WriteString(Config::sPathDebugLogLevelAvcodec, LogLevelAvcodec_toString(mSelectLogLevelAvcodec->getValue()).c_str());
        Config::WriteBool(Config::sPathDebugShowDebugInfoOnWidgets, mShowDebugInfoOnWidgets->IsChecked());
        Config::WriteBool(Config::sPathDebugLogSequenceOnEdit, mLogSequenceOnEdit->IsChecked());
        Config::WriteString(Config::sPathVideoDefaultFrameRate, (FrameRate::getSupported()[mFrameRate->GetSelection()]).toString());
        Config::WriteLong(Config::sPathVideoDefaultWidth, mDefaultVideoWidth->GetValue());
        Config::WriteLong(Config::sPathVideoDefaultHeight, mDefaultVideoHeight->GetValue());
        Config::WriteString(Config::sPathVideoDefaultScaling, model::VideoScaling_toString(mDefaultVideoScaling->getValue()).c_str());
        Config::WriteString(Config::sPathVideoDefaultAlignment, model::VideoAlignment_toString(mDefaultVideoAlignment->getValue()).c_str());
        Config::WriteLong(Config::sPathAudioDefaultSampleRate, toLong(mDefaultAudioSampleRate->GetValue()));
        Config::WriteLong(Config::sPathAudioDefaultNumberOfChannels, toLong(mDefaultAudioNumberOfChannels->GetValue()));
        Config::WriteBool(Config::sPathTimelineAutoAddEmptyTrackWhenDragging, mTimelineEnableAutoAddTracks->IsChecked());
        Config::WriteLong(Config::sPathTimelineMarkerBeginAddition, mMarkerBeginAddition->GetValue());
        Config::WriteLong(Config::sPathTimelineMarkerEndAddition, mMarkerEndAddition->GetValue());
        Config::WriteLong(Config::sPathTimelineDefaultStillImageLength, toLong(mDefaultStillImageLength->GetValue()));
        Config::WriteString(Config::sPathTimelineStripFromClipNames, mStrip->GetValue());

        wxString languageCode(Config::ReadString(Config::sPathWorkspaceLanguage));
        for (auto lang : getSupportedLanguages())
        {
            if (lang.first == mLanguage->GetStringSelection())
            {
                languageCode = lang.second;
                break;
            }
            mLanguage->Append(lang.first);
        }
        Config::WriteString(Config::sPathWorkspaceLanguage, languageCode);

        Config::releaseWriteToDisk();

        // Use new values
        Log::setReportingLevel(LogLevelConverter::readConfigValue(Config::sPathDebugLogLevel));
    }

    gui::Window::get().setDialogOpen(false);
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void DialogOptions::addtab(const wxString& name)
{
    mPanel = new wxPanel(GetBookCtrl(), wxID_ANY);
    GetBookCtrl()->AddPage(mPanel, name, true, GetBookCtrl()->GetPageCount());
    mTopSizer = new wxBoxSizer( wxVERTICAL );
    mPanel->SetSizerAndFit(mTopSizer);
    mBoxSizer = 0;
}

void DialogOptions::addbox(const wxString& name)
{
    ASSERT(mPanel);
    ASSERT(mTopSizer);
    mBoxSizer = new wxStaticBoxSizer(new wxStaticBox(mPanel, wxID_ANY, name), wxVERTICAL );
    mTopSizer->Add(mBoxSizer, 1, wxGROW|wxALL, 5 );
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

void DialogOptions::addnote(const wxString& text)
{
    wxStaticText* note = new wxStaticText(mPanel, wxID_ANY, text, wxDefaultPosition,wxDefaultSize, wxST_NO_AUTORESIZE);
    note->SetFont(note->GetFont().MakeItalic());
    note->Wrap(440);

    wxBoxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );
    mBoxSizer->Add(hSizer, 0, wxGROW|wxLEFT|wxALIGN_TOP|wxALL, 5);
    hSizer->Add(note, 1, wxALL|wxALIGN_TOP, 5);
}

} // namespace
