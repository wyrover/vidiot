// Copyright 2014-2015 Eric Raijmakers.
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

#include "EditProjectProperties.h"

#include "Folder.h"
#include "Project.h"
#include "ProjectModification.h"
#include "Properties.h"
#include "UtilAudioRate.h"
#include "UtilWindow.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

EditProjectProperties::EditProjectProperties(wxWindow* win)
    :   wxPanel(win)
    ,   mBoxSizer(0)
{
    SetSizer(new wxBoxSizer(wxVERTICAL));

    addbox(_("Video"));

    wxArrayString choices;
    for ( FrameRate fr : FrameRate::getSupported() )
    {
        choices.Add(fr.toString());
    }
    mVideoFrameRate = new wxRadioBox(this, wxID_ANY, wxT(""),wxPoint(10,10), wxDefaultSize, choices, 1, wxRA_SPECIFY_COLS );
    addoption(_("Framerate"), mVideoFrameRate);

    int initial = model::Properties::get().getVideoSize().GetWidth();
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
    sampleRateValidator.SetMin(getMinimumAudioSampleRate());
    sampleRateValidator.SetMax(getMaximumAudioSampleRate());
    initial = model::Properties::get().getAudioSampleRate();
    mAudioSampleRate = new wxComboBox(this, wxID_ANY, wxString::Format("%d", initial),  wxDefaultPosition, wxDefaultSize, sampleRateChoices, 0, sampleRateValidator);
    addoption(_("Audio sample rate"), mAudioSampleRate);

    wxIntegerValidator<int> channelValidator;
    channelValidator.SetMin(getMinimumNumberOfAudioChannels());
    channelValidator.SetMax(getMaximumNumberOfAudioChannels());
    wxArrayString channelChoices;
    channelChoices.Add("1");
    channelChoices.Add("2");
    initial = Config::ReadLong(Config::sPathAudioDefaultNumberOfChannels);
    mAudioNumberOfChannels = new wxComboBox(this, wxID_ANY, wxString::Format("%d", initial),  wxDefaultPosition, wxDefaultSize, channelChoices, 0, channelValidator);
    addoption(_("Audio channels"), mAudioNumberOfChannels);

    mBoxSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Note:")), wxVERTICAL );
    GetSizer()->Add(mBoxSizer, 0, wxALIGN_CENTRE|wxALL, 5 );
    mNote = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition,wxDefaultSize, wxST_NO_AUTORESIZE|wxALIGN_LEFT);
    mNote->SetFont(mNote->GetFont().MakeItalic());
    mNote->Wrap(300);
    wxBoxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );
    mBoxSizer->Add(hSizer, 0, wxGROW|wxLEFT|wxALL, 5);
    hSizer->Add(mNote, 0, wxRIGHT|wxALIGN_TOP, 5);

    read();
}

EditProjectProperties::~EditProjectProperties()
{
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

void EditProjectProperties::read()
{
    bool hasSequences = model::Project::get().getRoot()->hasSequences();

    unsigned int selection = 0;
    unsigned int count = 0;
    wxString currentFrameRate = model::Properties::get().getFrameRate().toString();
    for ( FrameRate fr : FrameRate::getSupported() )
    {
        if (currentFrameRate.IsSameAs(fr.toString()))
        {
            selection = count;
            break;
        }
        count++;
    }
    mVideoFrameRate->SetSelection(selection);
    mVideoFrameRate->Enable(!hasSequences);

    mVideoWidth->SetValue(model::Properties::get().getVideoSize().GetWidth());

    mVideoHeight->SetValue(model::Properties::get().getVideoSize().GetHeight());

    mAudioSampleRate->SetValue(wxString::Format("%d", model::Properties::get().getAudioSampleRate()));
    mAudioSampleRate->Enable(!hasSequences);

    mAudioNumberOfChannels->SetValue(wxString::Format("%d", model::Properties::get().getAudioNumberOfChannels()));

    mBoxSizer->Show(hasSequences);
    mNote->Show(hasSequences);
    if (hasSequences)
    {
        mNote->SetLabel(_("Some options cannot be changed because the project already contains sequences."));
    }

    Fit();
}

void EditProjectProperties::write()
{
    auto toLong = [](wxString comboBoxValue) -> long
    {
        long value(0);
        bool ok(false);
        ok = comboBoxValue.ToLong(&value);
        ASSERT(ok);
        return value;
    };
    // Disable 'undoing/redoing' of the creation of a sequence with the wrong (original) parameters - particularly frame rates
    model::Project::get().GetCommandProcessor()->ClearCommands();

    model::Properties::get().setFrameRate(FrameRate::getSupported()[mVideoFrameRate->GetSelection()]);
    model::Properties::get().setVideoSize(wxSize(mVideoWidth->GetValue(),mVideoHeight->GetValue()));
    model::Properties::get().setAudioSampleRate(toLong(mAudioSampleRate->GetValue()));
    model::Properties::get().setAudioNumberOfChannels(toLong(mAudioNumberOfChannels->GetValue()));
    model::ProjectModification::trigger();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void EditProjectProperties::addbox(const wxString& name)
{
    mBoxSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, name), wxVERTICAL );
    GetSizer()->Add(mBoxSizer, 0, wxGROW|wxALL, 5 );
}

void EditProjectProperties::addoption(const wxString& name, wxWindow* widget)
{
    ASSERT(mBoxSizer);
    wxBoxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );
    mBoxSizer->Add(hSizer, 0, wxGROW|wxLEFT|wxALL, 5);
    hSizer->Add(new wxStaticText(this, wxID_ANY, name), 0, wxALL|wxALIGN_TOP, 5);
    hSizer->Add(5, 5, 1, wxALL, 0);
    hSizer->Add(widget, 0, wxRIGHT|wxALIGN_TOP, 5);
}

} // namespace
