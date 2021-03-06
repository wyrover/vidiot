// Copyright 2014-2016 Eric Raijmakers.
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
#include "UtilWindow.h"
#include "Worker.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

DialogProjectProperties::DialogProjectProperties(wxWindow* win)
    :   wxDialog(win, wxID_ANY, model::Project::get().getName() + " " + _("Properties"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
    ,   mBoxSizer(0)
{
    util::window::setIcons(this);

    bool hasSequences = model::Project::get().getRoot()->hasSequences();
    bool activeTasks{ worker::VisibleWorker::get().isActive() };

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
    mVideoFrameRate->Enable(!hasSequences && !activeTasks);
    addoption(_("Framerate"), mVideoFrameRate);

    long initial = model::Properties::get().getVideoSize().GetWidth();
    mVideoWidth = new wxSpinCtrl(this, wxID_ANY, wxString::Format("%ld", initial), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, 20, 10000, initial);
    mVideoWidth->Enable(!activeTasks);
    addoption(_("Video width"), mVideoWidth);

    initial = model::Properties::get().getVideoSize().GetHeight();
    mVideoHeight = new wxSpinCtrl(this, wxID_ANY, wxString::Format("%ld", initial), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS | wxALIGN_RIGHT, 20, 10000, initial);
    mVideoHeight->Enable(!activeTasks);
    addoption(_("Video height"), mVideoHeight);

    addbox(_("Audio"));

    wxArrayString sampleRateChoices;
    sampleRateChoices.Add("22050");
    sampleRateChoices.Add("44100");
    sampleRateChoices.Add("48000");
    wxIntegerValidator<int> sampleRateValidator;
    sampleRateValidator.SetMin(1000);
    sampleRateValidator.SetMax(1000);
    initial = model::Properties::get().getAudioSampleRate();
    mAudioSampleRate = new wxComboBox(this, wxID_ANY, wxString::Format("%ld", initial),  wxDefaultPosition, wxDefaultSize, sampleRateChoices, 0, sampleRateValidator);
    mAudioSampleRate->Enable(!hasSequences && !activeTasks);
    addoption(_("Audio sample rate"), mAudioSampleRate);

    wxIntegerValidator<int> channelValidator;
    channelValidator.SetMin(1);
    channelValidator.SetMax(2);
    wxArrayString channelChoices;
    channelChoices.Add("1");
    channelChoices.Add("2");
    initial = Config::get().read<int>(Config::sPathAudioDefaultNumberOfChannels);
    mAudioNumberOfChannels = new wxComboBox(this, wxID_ANY, wxString::Format("%ld", initial),  wxDefaultPosition, wxDefaultSize, channelChoices, 0, channelValidator);
    mAudioNumberOfChannels->Enable(!activeTasks);
    addoption(_("Audio channels"), mAudioNumberOfChannels);

    if (hasSequences || activeTasks)
    {
        mBoxSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, _("Note:")), wxVERTICAL );
        GetSizer()->Add(mBoxSizer, 0, wxALIGN_CENTRE|wxALL, 5 );
        wxString sNote{ _("Some options cannot be changed because the project already contains sequences.") };
        if (activeTasks)
        {
            sNote = _("Can't change options now. They may be in use by the running background task(s).");
        }
        wxStaticText* note = new wxStaticText(this, wxID_ANY, sNote, wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE | wxALIGN_LEFT);
        note->SetFont(note->GetFont().MakeItalic());
        note->Wrap(300);
        wxBoxSizer* hSizer = new wxBoxSizer( wxHORIZONTAL );
        mBoxSizer->Add(hSizer, 0, wxGROW|wxLEFT|wxALL, 5);
        hSizer->Add(note, 0, wxRIGHT|wxALIGN_TOP, 5);
    }

    GetSizer()->Add(CreateButtonSizer(wxOK | wxCANCEL),0,wxALIGN_RIGHT);

    FindWindowById(wxID_OK)->Enable(!activeTasks);

    Fit();

    gui::Window::get().setDialogOpen(true);
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
        model::Properties::get().setAudioSampleRate(toLong(mAudioSampleRate->GetValue()));
        model::Properties::get().setAudioNumberOfChannels(toLong(mAudioNumberOfChannels->GetValue()));
        model::ProjectModification::trigger();
    }

    gui::Window::get().setDialogOpen(false);
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void DialogProjectProperties::addbox(const wxString& name)
{
    mBoxSizer = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, name), wxVERTICAL );
    GetSizer()->Add(mBoxSizer, 0, wxGROW|wxALL, 5 );
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
