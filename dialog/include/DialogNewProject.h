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

#ifndef DIALOG_NEW_PROJECT_H
#define DIALOG_NEW_PROJECT_H

#include "Project.h"

namespace model {
    class Project;
}

namespace gui {

class Pages;

class DialogNewProject
    :   public wxWizard
{
public:


    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    DialogNewProject(model::Project* project);
    virtual ~DialogNewProject();

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onActivateFolderText(wxMouseEvent& event);
    void onActivateFilesText(wxMouseEvent& event);
    void onActivateBlankText(wxMouseEvent& event);
    void onChangeType(wxCommandEvent& event);
    void onBrowseFolder(wxCommandEvent& event);
    void onBrowseFiles(wxCommandEvent& event);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::Project* mProject;

    wxWizardPageSimple* mStart;

    wxRadioButton* mButtonFolder;
    wxStaticText* mTextFolder;
    wxRadioButton* mButtonFiles;
    wxStaticText* mTextFiles;
    wxRadioButton* mButtonBlank; 
    wxStaticText* mTextBlank;

    wxWizardPageSimple* mFolder;
    wxButton* mButtonBrowseFolder;

    wxWizardPageSimple* mFiles;
    wxButton* mButtonBrowseFiles;

    wxWizardPageSimple* mBlank;
    wxWizardPageSimple* mProperties;

    //////////////////////////////////////////////////////////////////////////
    // HELPER CLASS
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Set the proper page order.
    void setLinks(); 

};

} // namespace

#endif
