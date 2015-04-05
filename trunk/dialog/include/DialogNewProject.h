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

#ifndef DIALOG_NEW_PROJECT_H
#define DIALOG_NEW_PROJECT_H

#include "Enums.h"
#include "UtilSingleInstance.h"

namespace model {
    class FileAnalyzer;
    class Project;
}

namespace gui {

class EditProjectProperties;

class DialogNewProject
    : public wxWizard
    , public SingleInstance<DialogNewProject>

{
public:


    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    DialogNewProject();
    virtual ~DialogNewProject();

    bool runWizard();

    static void setDroppedFiles(wxStrings files);

    //////////////////////////////////////////////////////////////////////////
    // GUI EVENTS
    //////////////////////////////////////////////////////////////////////////

    void onTypeChanged(wxCommandEvent& event);
    void onPageChanged(wxWizardEvent& event);
    void onCancel(wxWizardEvent& event);
    void onFinish(wxWizardEvent& event);
    void onActivateFolderText(wxMouseEvent& event);
    void onActivateFilesText(wxMouseEvent& event);
    void onActivateBlankText(wxMouseEvent& event);
    void onChangeType(wxCommandEvent& event);
    void onBrowseFolder(wxCommandEvent& event);
    void browseFolder();
    void handleFolder(wxString folder);
    void onBrowseFiles(wxCommandEvent& event);
    void browseFiles();
    void handleFiles(wxStrings files);

    //////////////////////////////////////////////////////////////////////////
    // TEST
    //////////////////////////////////////////////////////////////////////////

    wxWizardPage* getPageStart();
    wxWizardPage* getPageFolder();
    wxWizardPage* getPageFiles();
    wxWizardPage* getPageProperties();

    bool isNextEnabled() const;
    void pressNext();
    void pressCancel();
    void pressFinish();

    void pressButtonFolder();
    void pressButtonFiles();
    void pressButtonBlank();

    void pressBrowseFolder();
    void pressBrowseFiles();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxWizardPageSimple* mPageStart;
    model::DefaultNewProjectWizardStart mDefaultType;

    // Page: Choice of type of creation
    wxRadioButton* mButtonFolder;
    wxStaticText* mTextFolder;
    wxRadioButton* mButtonFiles;
    wxStaticText* mTextFiles;
    wxRadioButton* mButtonBlank; 
    wxStaticText* mTextBlank;

    // Page: From folder
    wxWizardPageSimple* mPageFolder;
    wxButton* mButtonBrowseFolder;
    wxTextCtrl* mContentsFolder;
    wxString mFolderPath;

    // Page: From files
    wxWizardPageSimple* mPageFiles;
    wxButton* mButtonBrowseFiles;
    wxStaticText* mContentsFiles;
    wxStrings mFilePaths;

    // Page: Properties
    wxWizardPageSimple* mPageProperties;
    EditProjectProperties* mEditProperties;

    static boost::optional<wxStrings> sDroppedFiles;

    boost::shared_ptr<model::FileAnalyzer> mFileAnalyzer;

    //////////////////////////////////////////////////////////////////////////
    // HELPER CLASS
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Set the proper page order.
    void setLinks(); 

    void showFoundFilesInFolder();

    void showSelectedFiles();

    wxString getOverviewMessage(boost::shared_ptr<model::FileAnalyzer> analyzer) const;
};

} // namespace

#endif
