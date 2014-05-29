// Copyright 2014 Eric Raijmakers.
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

#include "DialogProjectProperties.h"

#include "Folder.h"
#include "Project.h"
#include "ProjectModification.h"
#include "Properties.h"
#include "UtilLog.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

DialogProjectProperties::DialogProjectProperties(wxWindow* win)
    :   wxDialog(win, wxID_ANY, model::Project::get().getName() + " " + _("Properties"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
    ,   mBoxSizer(0)
{
    wxIconBundle icons; // todo make helper for geticonbundle
    wxFileName iconfile(Config::getExeDir(),"movie_all.ico"); // todo extend utilpath to make filenames directly (with multiple path parts)
    iconfile.AppendDir("icons");
    icons.AddIcon(iconfile.GetFullPath()); // Icon in title bar of window
    SetIcons(icons);

    bool hasSequences = model::Project::get().getRoot()->hasSequences();

    SetSizer(new wxBoxSizer(wxVERTICAL));

    addbox(_("Video"));

    wxArrayString choices;
    unsigned int selection = 0;
    wxString currentFrameRate = model::Properties::get().getFrameRate().toString();
    for ( FrameRate fr : FrameRate::getSupported() )
    {
        wxString frs = fr.toString();
        choices.Add(frs);
        if (currentFrameRate.IsSameAs(frs))
        {
            selection = choices.GetCount() - 1;
        }
    }
    mVideoFrameRate = new wxRadioBox(this, wxID_ANY, wxT(""),wxPoint(10,10), wxDefaultSize, choices, 1, wxRA_SPECIFY_COLS );
    mVideoFrameRate->SetSelection(selection);
    mVideoFrameRate->Enable(!hasSequences);
    addoption(_("Framerate"), mVideoFrameRate);

    long initial = model::Properties::get().getVideoSize().GetWidth();
    mVideoWidth = new wxSpinCtrl(this, wxID_ANY, wxString::Format("%d", initial), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, 20, 10000, initial);
    addoption(_("Video width"), mVideoWidth);

    initial = model::Properties::get().getVideoSize().GetHeight();
    mVideoHeight = new wxSpinCtrl(this, wxID_ANY, wxString::Format("%d", initial), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, 20, 10000, initial);
    addoption(_("Video height"), mVideoHeight);

    addbox(_("Audio"));

    wxArrayString sampleRateChoices;
    sampleRateChoices.Add("22050");
    sampleRateChoices.Add("44100");
    sampleRateChoices.Add("48000");
    wxIntegerValidator<int> sampleRateValidator;
    sampleRateValidator.SetMin(1000);
    sampleRateValidator.SetMax(1000);
    initial = model::Properties::get().getAudioFrameRate();
    mAudioSampleRate = new wxComboBox(this, wxID_ANY, wxString::Format("%d", initial),  wxDefaultPosition, wxDefaultSize, sampleRateChoices, 0, sampleRateValidator);
    mAudioSampleRate->Enable(!hasSequences);
    addoption(_("Audio sample rate"), mAudioSampleRate);

    wxIntegerValidator<int> channelValidator;
    channelValidator.SetMin(1);
    channelValidator.SetMax(2);
    wxArrayString channelChoices;
    channelChoices.Add("1");
    channelChoices.Add("2");
    initial = Config::ReadLong(Config::sPathDefaultAudioChannels);
    mAudioNumberOfChannels = new wxComboBox(this, wxID_ANY, wxString::Format("%d", initial),  wxDefaultPosition, wxDefaultSize, channelChoices, 0, channelValidator);
    addoption(_("Audio channels"), mAudioNumberOfChannels);

    if (hasSequences)
    {
        mBoxSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Note:")), wxVERTICAL );
        GetSizer()->Add(mBoxSizer, 0, wxALIGN_CENTRE|wxALL, 5 );
        wxStaticText* note = new wxStaticText(this, wxID_ANY, "Some options cannot be changed because the project already contains sequences.", wxDefaultPosition,wxDefaultSize, wxST_NO_AUTORESIZE|wxALIGN_LEFT);
        note->SetFont(note->GetFont().MakeItalic());
        note->Wrap(300);
        wxBoxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );
        mBoxSizer->Add(hSizer, 0, wxGROW|wxLEFT|wxALL, 5);
        hSizer->Add(note, 0, wxRIGHT|wxALIGN_TOP, 5);
    }

    GetSizer()->Add(CreateButtonSizer(wxOK | wxCANCEL),0,wxALIGN_RIGHT);

    Fit();

}

DialogProjectProperties::~DialogProjectProperties()
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
        // Disable 'undoing/redoing' of the creation of a sequence with the wrong (original) parameters - particularly frame rates
        model::Project::get().GetCommandProcessor()->ClearCommands();

        model::Properties::get().setFrameRate(FrameRate::getSupported()[mVideoFrameRate->GetSelection()]);
        model::Properties::get().setVideoSize(wxSize(mVideoWidth->GetValue(),mVideoHeight->GetValue()));
        model::Properties::get().setAudioFrameRate(toLong(mAudioSampleRate->GetValue()));
        model::Properties::get().setAudioNumberOfChannels(toLong(mAudioNumberOfChannels->GetValue()));
        model::ProjectModification::trigger();
    }
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void DialogProjectProperties::addbox(const wxString& name)
{
    mBoxSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, name), wxVERTICAL );
    GetSizer()->Add(mBoxSizer, 0, wxGROW|wxALIGN_CENTRE|wxALL, 5 );
}

void DialogProjectProperties::addoption(const wxString& name, wxWindow* widget)
{
    ASSERT(mBoxSizer);
    wxBoxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );
    mBoxSizer->Add(hSizer, 0, wxGROW|wxLEFT|wxALL, 5);
    hSizer->Add(new wxStaticText(this, wxID_ANY, name), 0, wxALL|wxALIGN_TOP, 5);
    hSizer->Add(5, 5, 1, wxALL, 0);
    hSizer->Add(widget, 0, wxRIGHT|wxALIGN_TOP, 5);
}

} // namespace
