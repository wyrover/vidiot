// Copyright 2013,2014 Eric Raijmakers.
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

#include "DialogNewProject.h"

#include "Project.h"
#include "Window.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

DialogNewProject::DialogNewProject(model::Project* project)
    :   wxWizard(&Window::get())
    ,   mProject(project)
{
    SetTitle(_("Create new project"));
    
    {
        mStart = new wxWizardPageSimple(this);

        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

        wxBoxSizer* sizerFolder = new wxBoxSizer(wxHORIZONTAL);
        mButtonFolder = new wxRadioButton(mStart, wxID_ANY,  _(""), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
        mTextFolder = new wxStaticText(mStart, wxID_ANY, _("Start a new project starting from a folder of files.\nAll files in this folder are added into a new movie."));
        sizerFolder->Add(mButtonFolder, wxSizerFlags(0).Expand().Border(wxRIGHT,5));
        sizerFolder->Add(mTextFolder, wxSizerFlags(1).Center());
        mButtonFolder->Bind(wxEVT_RADIOBUTTON, &DialogNewProject::onChangeType, this);
        mTextFolder->Bind(wxEVT_LEFT_DOWN, &DialogNewProject::onActivateFolderText,this);

        wxBoxSizer* sizerFiles = new wxBoxSizer(wxHORIZONTAL);
        mButtonFiles = new wxRadioButton(mStart, wxID_ANY, "");
        mTextFiles = new wxStaticText(mStart, wxID_ANY, _("Start a new project by selecting a list of files\nto be added to a new movie."), wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE);
        sizerFiles->Add(mButtonFiles, wxSizerFlags(0).Expand().Border(wxRIGHT,5));
        sizerFiles->Add(mTextFiles, wxSizerFlags(1).Center());
        sizerFiles->Fit(mStart);
        mButtonFiles->Bind(wxEVT_RADIOBUTTON, &DialogNewProject::onChangeType, this);
        mTextFiles->Bind(wxEVT_LEFT_DOWN, &DialogNewProject::onActivateFilesText,this);

        wxBoxSizer* sizerBlank = new wxBoxSizer(wxHORIZONTAL);
        mButtonBlank = new wxRadioButton(mStart, wxID_ANY, "");
        mTextBlank = new wxStaticText(mStart, wxID_ANY, _("Start a new empty project."));
        sizerBlank->Add(mButtonBlank, wxSizerFlags(0).Expand().Border(wxRIGHT,5));
        sizerBlank->Add(mTextBlank, wxSizerFlags(1).Center());
        mButtonBlank->Bind(wxEVT_RADIOBUTTON, &DialogNewProject::onChangeType, this);
        mTextBlank->Bind(wxEVT_LEFT_DOWN, &DialogNewProject::onActivateBlankText,this);

        sizer->Add(sizerFolder, wxSizerFlags(1));
        sizer->Add(new wxStaticLine(mStart), wxSizerFlags(0).Expand());
        sizer->Add(sizerFiles, wxSizerFlags(1).Expand());
        sizer->Add(new wxStaticLine(mStart), wxSizerFlags(0).Expand());
        sizer->Add(sizerBlank, wxSizerFlags(1).Expand());


        mStart->SetSizer(sizer);
    }
    {
        mBlank = new wxWizardPageSimple(this);

    }
    {
        mFolder = new wxWizardPageSimple(this);

        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

        wxStaticText* textBrowse = new wxStaticText(mFolder, wxID_ANY, _("Select a folder with media files\nfor creating a new movie."));
        sizer->Add(textBrowse,wxSizerFlags(0).Expand());

        wxBoxSizer* sizerFolder = new wxBoxSizer(wxHORIZONTAL);
        mButtonBrowseFolder = new wxButton(mFolder, wxID_ANY, _("Browse folder"));
        wxStaticText* textFolder = new wxStaticText(mFolder, wxID_ANY, "");
        sizerFolder->Add(mButtonBrowseFolder, wxSizerFlags(0).Border(wxRIGHT,5));
        sizerFolder->Add(textFolder, wxSizerFlags(1).Center());
        mButtonBrowseFolder->Bind(wxEVT_BUTTON, &DialogNewProject::onBrowseFolder, this);

        sizer->Add(sizerFolder,wxSizerFlags(1).Expand());

        // todo disable next until valid folder selected

        mFolder->SetSizer(sizer);
    }
    {
        mFiles = new wxWizardPageSimple(this);

        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

        wxStaticText* textBrowse = new wxStaticText(mFiles, wxID_ANY, _("Select media files to be added to the movie."));
        sizer->Add(textBrowse,wxSizerFlags(0).Expand());

        wxBoxSizer* sizerFiles = new wxBoxSizer(wxHORIZONTAL);
        mButtonBrowseFiles = new wxButton(mFiles, wxID_ANY, _("Browse files"));
        wxStaticText* textFiles = new wxStaticText(mFiles, wxID_ANY, "");
        sizerFiles->Add(mButtonBrowseFiles, wxSizerFlags(0).Border(wxRIGHT,5));
        sizerFiles->Add(textFiles, wxSizerFlags(1).Center());
        mButtonBrowseFiles->Bind(wxEVT_BUTTON, &DialogNewProject::onBrowseFiles, this);

        sizer->Add(sizerFiles,wxSizerFlags(1).Expand());

        // todo disable next until valid files selected

        mFiles->SetSizer(sizer);

    }
    {
        mProperties = new wxWizardPageSimple(this);
    }

    setLinks();
    RunWizard(mStart);
}

DialogNewProject::~DialogNewProject()
{
    mTextFolder->Unbind(wxEVT_LEFT_DOWN, &DialogNewProject::onActivateFolderText,this);
    mTextFiles->Unbind(wxEVT_LEFT_DOWN, &DialogNewProject::onActivateFilesText,this);
    mTextBlank->Unbind(wxEVT_LEFT_DOWN, &DialogNewProject::onActivateBlankText,this);
    mButtonBrowseFolder->Unbind(wxEVT_BUTTON, &DialogNewProject::onBrowseFolder, this);
    mButtonBrowseFiles->Unbind(wxEVT_BUTTON, &DialogNewProject::onBrowseFiles, this);
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void DialogNewProject::onActivateFolderText(wxMouseEvent& event)
{
    mButtonFolder->SetValue(true);
    setLinks();
    event.Skip();
}

void DialogNewProject::onActivateFilesText(wxMouseEvent& event)
{
    mButtonFiles->SetValue(true);
    setLinks();
    event.Skip();
}

void DialogNewProject::onActivateBlankText(wxMouseEvent& event)
{
    mButtonBlank->SetValue(true);
    setLinks();
    event.Skip();
}

void DialogNewProject::onChangeType(wxCommandEvent& event)
{
    setLinks();
    event.Skip();
}

void DialogNewProject::onBrowseFolder(wxCommandEvent& event)
{
    wxString s = gui::Dialog::get().getDir( _("Select folder with media files"),wxStandardPaths::Get().GetDocumentsDir() );
    event.Skip();
}


void DialogNewProject::onBrowseFiles(wxCommandEvent& event)
{
    wxStrings s = gui::Dialog::get().getFiles( _("Select media files") );
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void DialogNewProject::setLinks()
{
    mStart->SetPrev(nullptr);
    mStart->SetNext(
        mButtonBlank->GetValue() ? mBlank :
        mButtonFiles->GetValue() ? mFiles :
        mButtonFolder->GetValue() ? mFolder : nullptr);

    mFolder->SetPrev(mStart);
    mFolder->SetNext(mProperties);

    mFiles->SetPrev(mStart);
    mFiles->SetNext(mProperties);

    mBlank->SetPrev(mStart);
    mBlank->SetNext(mProperties);

    mProperties->SetPrev(
        mButtonBlank->GetValue() ? mBlank :
        mButtonFiles->GetValue() ? mFiles :
        mButtonFolder->GetValue() ? mFolder : nullptr);
    mProperties->SetNext(nullptr);
}

}