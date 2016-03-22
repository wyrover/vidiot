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

#include "DialogNewProject.h"

#include "AutoFolder.h"
#include "EditProjectProperties.h"
#include "FileAnalyzer.h"
#include "Project.h"
#include "ProjectModification.h"
#include "ProjectView.h"
#include "ProjectViewCreateSequence.h"
#include "Properties.h"
#include "Render.h"
#include "Sequence.h"
#include "UtilAudioRate.h"
#include "UtilEnum.h"
#include "Window.h"

namespace gui {

// static
boost::optional<wxStrings> DialogNewProject::sDroppedFiles = boost::none;

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

DialogNewProject::DialogNewProject()
    : wxWizard()
    , mDefaultType(Config::get().ReadEnum<model::DefaultNewProjectWizardStart>(Config::sPathProjectDefaultNewProjectType))
    , mFolderPath("")
    , sTitle(_("Create new project"))
    , sNoFiles(_("No media files found."))
    , sFolder(_("Select a folder containing media files for creating a new movie."))
    , sFiles(_("Select individual media files for creating a new movie."))
    , sEmpty(_("Start a new empty project."))
    , sFolderShort(_("Select folder"))
    , sFilesShort(_("Select files"))
{
    
    Create(&Window::get(), wxID_ANY, sTitle, util::window::getBitmap("movie128.png"), wxDefaultPosition, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    util::window::setIcons(this);
    SetTitle(sTitle);

    wxSize minPageSize(0,0);

    {
        // Page: Choice of type of creation
        mPageStart = new wxWizardPageSimple(this);

        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

        wxBoxSizer* sizerFolder = new wxBoxSizer(wxHORIZONTAL);
        mButtonFolder = new wxRadioButton(mPageStart, wxID_ANY,  "", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
        mTextFolder = new wxStaticText(mPageStart, wxID_ANY, sFolder);
        sizerFolder->Add(mButtonFolder, wxSizerFlags(0).Expand().Border(wxRIGHT,5));
        sizerFolder->Add(mTextFolder, wxSizerFlags(1).Center());
        mButtonFolder->Bind(wxEVT_RADIOBUTTON, &DialogNewProject::onChangeType, this);
        mTextFolder->Bind(wxEVT_LEFT_DOWN, &DialogNewProject::onActivateFolderText,this);

        wxBoxSizer* sizerFiles = new wxBoxSizer(wxHORIZONTAL);
        mButtonFiles = new wxRadioButton(mPageStart, wxID_ANY, "");
        mTextFiles = new wxStaticText(mPageStart, wxID_ANY, sFiles, wxDefaultPosition, wxDefaultSize, wxST_NO_AUTORESIZE);
        sizerFiles->Add(mButtonFiles, wxSizerFlags(0).Expand().Border(wxRIGHT,5));
        sizerFiles->Add(mTextFiles, wxSizerFlags(1).Center());
        sizerFiles->Fit(mPageStart);
        mButtonFiles->Bind(wxEVT_RADIOBUTTON, &DialogNewProject::onChangeType, this);
        mTextFiles->Bind(wxEVT_LEFT_DOWN, &DialogNewProject::onActivateFilesText,this);

        wxBoxSizer* sizerBlank = new wxBoxSizer(wxHORIZONTAL);
        mButtonBlank = new wxRadioButton(mPageStart, wxID_ANY, "");
        mTextBlank = new wxStaticText(mPageStart, wxID_ANY, sEmpty);
        sizerBlank->Add(mButtonBlank, wxSizerFlags(0).Expand().Border(wxRIGHT,5));
        sizerBlank->Add(mTextBlank, wxSizerFlags(1).Center());
        mButtonBlank->Bind(wxEVT_RADIOBUTTON, &DialogNewProject::onChangeType, this);
        mTextBlank->Bind(wxEVT_LEFT_DOWN, &DialogNewProject::onActivateBlankText,this);

        sizer->Add(sizerFolder, wxSizerFlags(1));
        sizer->Add(new wxStaticLine(mPageStart), wxSizerFlags(0).Expand());
        sizer->Add(sizerFiles, wxSizerFlags(1).Expand());
        sizer->Add(new wxStaticLine(mPageStart), wxSizerFlags(0).Expand());
        sizer->Add(sizerBlank, wxSizerFlags(1).Expand());

        mPageStart->SetSizer(sizer);

        minPageSize.IncTo(sizer->CalcMin());

    }
    static const int sBorderSize(10);
    {
        // Page: From folder
        mPageFolder = new wxWizardPageSimple(this);

        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

        wxStaticText* textBrowse = new wxStaticText(mPageFolder, wxID_ANY, sFolder);
        textBrowse->Wrap(400);
        sizer->Add(textBrowse,wxSizerFlags(0).Expand().Border(wxBOTTOM, sBorderSize));

        mButtonBrowseFolder = new wxButton(mPageFolder, wxID_ANY, sFolderShort);
        mContentsFolder = new wxTextCtrl(mPageFolder, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_MULTILINE | wxTE_NO_VSCROLL | wxBORDER_NONE);
        mContentsFolder->SetFont(mContentsFolder->GetFont().MakeItalic());
        mContentsFolder->SetBackgroundColour(mPageFolder->GetBackgroundColour());

        sizer->Add(mButtonBrowseFolder,wxSizerFlags(0).Border(wxBOTTOM, sBorderSize));
        sizer->Add(mContentsFolder,wxSizerFlags(1).Expand());

        mPageFolder->SetSizer(sizer);
        minPageSize.IncTo(sizer->CalcMin());

        showFoundFilesInFolder();
    }
    {
        // Page: From files
        mPageFiles = new wxWizardPageSimple(this);

        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

        wxStaticText* textBrowse = new wxStaticText(mPageFiles, wxID_ANY, sFiles);
        textBrowse->Wrap(400);
        sizer->Add(textBrowse,wxSizerFlags(0).Expand().Border(wxBOTTOM, sBorderSize));

        mButtonBrowseFiles = new wxButton(mPageFiles, wxID_ANY, sFilesShort);
        mContentsFiles = new wxStaticText(mPageFiles, wxID_ANY, "");
        mContentsFiles->SetFont(mContentsFiles->GetFont().MakeItalic());

        sizer->Add(mButtonBrowseFiles,wxSizerFlags(0).Border(wxBOTTOM, sBorderSize));
        sizer->Add(mContentsFiles,wxSizerFlags(1).Expand());

        mPageFiles->SetSizer(sizer);
        minPageSize.IncTo(sizer->CalcMin());

        showSelectedFiles();
    }
    {
        // Page: Properties
        mPageProperties = new wxWizardPageSimple(this);

        wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

        mEditProperties = new EditProjectProperties(mPageProperties);

        sizer->Add(mEditProperties,wxSizerFlags(1).Expand());

        mPageProperties->SetSizer(sizer);
        minPageSize.IncTo(sizer->CalcMin());
    }

    Bind(wxEVT_WIZARD_PAGE_CHANGED, &DialogNewProject::onPageChanged, this);
    Bind(wxEVT_WIZARD_CANCEL, &DialogNewProject::onCancel, this);
    Bind(wxEVT_WIZARD_FINISHED, &DialogNewProject::onFinish, this);

    mButtonFolder->Bind(wxEVT_RADIOBUTTON, &DialogNewProject::onTypeChanged, this);
    mButtonFiles->Bind(wxEVT_RADIOBUTTON, &DialogNewProject::onTypeChanged, this);
    mButtonBlank->Bind(wxEVT_RADIOBUTTON, &DialogNewProject::onTypeChanged, this);

    mButtonBrowseFolder->Bind(wxEVT_BUTTON, &DialogNewProject::onBrowseFolder, this);
    mButtonBrowseFiles->Bind(wxEVT_BUTTON, &DialogNewProject::onBrowseFiles, this);

    if (sDroppedFiles)
    {
        // Files were dropped from the file system.
        mFileAnalyzer = boost::make_shared<model::FileAnalyzer>(*sDroppedFiles, this);

        if (sDroppedFiles->size() == 1 &&
            mFileAnalyzer->getNumberOfFolders() == 1)
        {
            mDefaultType = model::DefaultNewProjectWizardStartFolder;
            handleFolder(sDroppedFiles->front());
        }
        else if (mFileAnalyzer->getNumberOfFolders() + mFileAnalyzer->getNumberOfMediaFiles() >= 1)
        {
            mDefaultType = model::DefaultNewProjectWizardStartFiles;
            handleFiles(*sDroppedFiles);
        }
        else
        {
            mDefaultType = model::DefaultNewProjectWizardStartBlank;
        }

        sDroppedFiles.reset();
    }

    switch (mDefaultType)
    {
    case model::DefaultNewProjectWizardStartNone:
        break;
    case model::DefaultNewProjectWizardStartFolder:
        mButtonFolder->SetValue(true);
        break;
    case model::DefaultNewProjectWizardStartFiles:
        mButtonFiles->SetValue(true);
        break;
    case model::DefaultNewProjectWizardStartBlank:
        mButtonBlank->SetValue(true);
        break;
    }

    setLinks();
    SetPageSize(minPageSize);

    gui::Window::get().setDialogOpen(true);
}

DialogNewProject::~DialogNewProject()
{
    mTextFolder->Unbind(wxEVT_LEFT_DOWN, &DialogNewProject::onActivateFolderText,this);
    mTextFiles->Unbind(wxEVT_LEFT_DOWN, &DialogNewProject::onActivateFilesText,this);
    mTextBlank->Unbind(wxEVT_LEFT_DOWN, &DialogNewProject::onActivateBlankText,this);
    mButtonFolder->Unbind(wxEVT_RADIOBUTTON, &DialogNewProject::onTypeChanged, this);
    mButtonFiles->Unbind(wxEVT_RADIOBUTTON, &DialogNewProject::onTypeChanged, this);
    mButtonBlank->Unbind(wxEVT_RADIOBUTTON, &DialogNewProject::onTypeChanged, this);
    mButtonBrowseFolder->Unbind(wxEVT_BUTTON, &DialogNewProject::onBrowseFolder, this);
    mButtonBrowseFiles->Unbind(wxEVT_BUTTON, &DialogNewProject::onBrowseFiles, this);
    Unbind(wxEVT_WIZARD_PAGE_CHANGED, &DialogNewProject::onPageChanged, this);
    Unbind(wxEVT_WIZARD_CANCEL, &DialogNewProject::onCancel, this);
    Unbind(wxEVT_WIZARD_FINISHED, &DialogNewProject::onFinish, this);

    gui::Window::get().setDialogOpen(false);
}

// static
bool DialogNewProject::runWizard()
{
    if (Config::get().ReadEnum<model::DefaultNewProjectWizardStart>(Config::sPathProjectDefaultNewProjectType) == model::DefaultNewProjectWizardStartNone)
    {
        return true;
    }
    return DialogNewProject().RunWizard();
}

bool DialogNewProject::RunWizard(wxWizardPage* firstPage)
{
    if (firstPage == nullptr)
    {
        return wxWizard::RunWizard(mPageStart);
    }
    return wxWizard::RunWizard(firstPage);
}

// static
void DialogNewProject::setDroppedFiles(wxStrings files)
{
    sDroppedFiles.reset(files);
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void DialogNewProject::onTypeChanged(wxCommandEvent& event)
{
    setLinks();
    event.Skip();
}

void DialogNewProject::onPageChanged(wxWizardEvent& event)
{
    setLinks();
    if (event.GetPage() == mPageProperties)
    {
        if (mFileAnalyzer)
        {
            wxSize size = mFileAnalyzer->getMostFrequentVideoSize();
            model::Properties::get().setVideoSize(size);

            FrameRate fr = mFileAnalyzer->getMostFrequentFrameRate();
            std::vector<FrameRate> supported = FrameRate::getSupported();
            if (std::find(supported.begin(), supported.end(), mFileAnalyzer->getMostFrequentFrameRate()) != supported.end())
            {
                // Only use supported framerates
                model::Properties::get().setFrameRate(fr);
            }
            // else: Fall back to default frame rate

            std::pair<int,int> sampleRate_channels = mFileAnalyzer->getMostFrequentAudioRate();
            if (sampleRate_channels.second >= getMinimumNumberOfAudioChannels() && sampleRate_channels.second <= getMaximumNumberOfAudioChannels())
            {
                // Number of channels ok
                if (sampleRate_channels.first >= getMinimumAudioSampleRate() && sampleRate_channels.first <= getMaximumAudioSampleRate())
                {
                    // Sample rate also ok
                    model::Properties::get().setAudioSampleRate(sampleRate_channels.first);
                    model::Properties::get().setAudioNumberOfChannels(sampleRate_channels.second);
                }
            }
        }
        mEditProperties->read();
    }
    else if ((event.GetPage() == mPageFolder) && mFolderPath.IsEmpty())
    {
        browseFolder();
    }
    else if ((event.GetPage() == mPageFiles) && mFilePaths.empty())
    {
        browseFiles();
    }
    event.Skip();
}

void DialogNewProject::onCancel(wxWizardEvent& event)
{
    event.Skip();
}

void DialogNewProject::onFinish(wxWizardEvent& event)
{
    mEditProperties->write();
    model::DefaultNewProjectWizardStart defaultType =
        mButtonFolder->GetValue() ? model::DefaultNewProjectWizardStartFolder :
        mButtonFiles->GetValue() ? model::DefaultNewProjectWizardStartFiles :
        model::DefaultNewProjectWizardStartBlank;
    Config::get().write<wxString>(Config::sPathProjectDefaultNewProjectType, model::DefaultNewProjectWizardStart_toString(defaultType));

    if (mFileAnalyzer)
    {
        model::NodePtrs nodes = mFileAnalyzer->getNodes();
        model::FolderPtr root = ::model::Project::get().getRoot();

        // Expand root immediately after opening the project
        ProjectView::get().setOpenFolders({ root });

        mFileAnalyzer->addNodesToProjectView();

        if (mFileAnalyzer->getNumberOfFolders() == 1 && mFileAnalyzer->getNumberOfFiles() == 0)
        {
            // Create sequence of this folder
            ::model::AutoFolderPtr folder = boost::dynamic_pointer_cast<::model::AutoFolder>(nodes.front());
            ASSERT(folder);
            model::ProjectModification::submit(new cmd::ProjectViewCreateSequence(folder));
        }
        else if (mFileAnalyzer->getNumberOfFolders() == 0 && mFileAnalyzer->getNumberOfMediaFiles() > 0)
        {
            // Create sequence of all given files
            
            // Note: INode->getName may contain a full path to a file.
            wxFileName firstFileName{ util::path::toFileName(nodes.front()->getName()) };

            wxString sequenceName = nodes.size() > 1 ? _("Movie") : firstFileName.GetName();
            cmd::ProjectViewCreateSequence* cmd{ new cmd::ProjectViewCreateSequence(root, sequenceName, nodes) };
            model::ProjectModification::submit(cmd);

            // Make the rendered output path relative to the input file
            wxFileName renderFilename{ cmd->getSequence()->getRender()->getFileName() };
            firstFileName.SetFullName(renderFilename.GetFullName());
            cmd->getSequence()->getRender()->setFileName(firstFileName);
        }
    }

    event.Skip();
}

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
    browseFolder();
    event.Skip();
}

void DialogNewProject::browseFolder()
{
    wxString selection = gui::Dialog::get().getDir(sFolder,wxStandardPaths::Get().GetDocumentsDir());
    if (!selection.IsEmpty())
    {
        mFileAnalyzer.reset();
        handleFolder(selection);
    }
}

void DialogNewProject::handleFolder(wxString folder)
{
    mFolderPath = folder;
    showFoundFilesInFolder();
    setLinks();
}

void DialogNewProject::onBrowseFiles(wxCommandEvent& event)
{
    browseFiles();
    event.Skip();
}

void DialogNewProject::browseFiles()
{
    wxStrings selection = gui::Dialog::get().getFiles(sFiles);
    if (!selection.empty())
    {
        mFileAnalyzer.reset();
        handleFiles(selection);
    }
}

void DialogNewProject::handleFiles(wxStrings files)
{
    mFilePaths = files;
    showSelectedFiles();
    setLinks();
}

//////////////////////////////////////////////////////////////////////////
// TEST
//////////////////////////////////////////////////////////////////////////

wxWizardPage* DialogNewProject::getPageStart()
{
    return mPageStart;
}

wxWizardPage* DialogNewProject::getPageFolder()
{
    return mPageFolder;
}

wxWizardPage* DialogNewProject::getPageFiles()
{
    return mPageFiles;
}

wxWizardPage* DialogNewProject::getPageProperties()
{
    return mPageProperties;
}

bool DialogNewProject::isNextEnabled() const
{
    wxButton* nextButton = dynamic_cast<wxButton*>(FindWindowById(wxID_FORWARD));
    return nextButton->IsEnabled();
}

void DialogNewProject::pressNext()
{
    wxButton* nextButton = dynamic_cast<wxButton*>(FindWindowById(wxID_FORWARD));
    wxCommandEvent* event = new wxCommandEvent(wxEVT_BUTTON, wxID_FORWARD);
    event->SetEventObject(nextButton);
    nextButton->GetEventHandler()->QueueEvent(event);
}

void DialogNewProject::pressCancel()
{
    wxButton* cancelButton = dynamic_cast<wxButton*>(FindWindowById(wxID_CANCEL));
    wxCommandEvent* event = new wxCommandEvent(wxEVT_BUTTON, wxID_CANCEL);
    event->SetEventObject(cancelButton);
    cancelButton->GetEventHandler()->QueueEvent(event);
}

void DialogNewProject::pressFinish()
{
    wxButton* finishButton = dynamic_cast<wxButton*>(FindWindowById(wxID_FORWARD));
    ASSERT_EQUALS(finishButton->GetLabel(),_("&Finish"));
    pressNext();
}

void DialogNewProject::pressButtonFolder()
{
    mButtonFolder->SetValue(true);
    wxCommandEvent* event = new wxCommandEvent(wxEVT_RADIOBUTTON);
    event->SetEventObject(mButtonFolder);
    mButtonFolder->GetEventHandler()->QueueEvent(event);
}

void DialogNewProject::pressButtonFiles()
{
    mButtonFiles->SetValue(true);
    wxCommandEvent* event = new wxCommandEvent(wxEVT_RADIOBUTTON);
    event->SetEventObject(mButtonFiles);
    mButtonFiles->GetEventHandler()->QueueEvent(event);
}

void DialogNewProject::pressButtonBlank()
{
    mButtonBlank->SetValue(true);
    wxCommandEvent* event = new wxCommandEvent(wxEVT_RADIOBUTTON);
    event->SetEventObject(mButtonBlank);
    mButtonBlank->GetEventHandler()->QueueEvent(event);
}

void DialogNewProject::pressBrowseFolder()
{
    wxCommandEvent* event = new wxCommandEvent(wxEVT_BUTTON);
    mButtonBrowseFolder->GetEventHandler()->QueueEvent(event);
}

void DialogNewProject::pressBrowseFiles()
{
    wxCommandEvent* event = new wxCommandEvent(wxEVT_BUTTON);
    mButtonBrowseFiles->GetEventHandler()->QueueEvent(event);
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void DialogNewProject::setLinks()
{
    mPageStart->SetPrev(nullptr);
    mPageStart->SetNext(
        mButtonBlank->GetValue() ? mPageProperties :
        mButtonFiles->GetValue() ? mPageFiles :
        mButtonFolder->GetValue() ? mPageFolder : nullptr);

    mPageFolder->SetPrev(mPageStart);
    mPageFolder->SetNext(mPageProperties);

    mPageFiles->SetPrev(mPageStart);
    mPageFiles->SetNext(mPageProperties);

    wxWindow* nextButton = FindWindowById(wxID_FORWARD);
    nextButton->Enable();
    if (GetCurrentPage() == mPageFolder && mFolderPath.IsEmpty())
    {
        nextButton->Disable();
    }
    if (GetCurrentPage() == mPageFiles && mFilePaths.empty())
    {
        nextButton->Disable();
    }

    mPageProperties->SetPrev(
        mButtonBlank->GetValue() ? mPageStart :
        mButtonFiles->GetValue() ? mPageFiles :
        mButtonFolder->GetValue() ? mPageFolder : nullptr);
    mPageProperties->SetNext(nullptr);
}

void DialogNewProject::showFoundFilesInFolder()
{
    wxString overview;
    if (mFolderPath.IsEmpty())
    {
        overview << sNoFiles;
    }
    else
    {
        if (!mFileAnalyzer)
        {
            wxStrings paths = { mFolderPath };
            mFileAnalyzer = boost::make_shared<model::FileAnalyzer>(paths,this);
        }
        // else: Dialog canceled or FileAnalyzer already initialized by drag and drop

        overview = getOverviewMessage(mFileAnalyzer);
        if (overview.IsSameAs(sNoFiles))
        {
            mFolderPath = "";
        }
    }
    mContentsFolder->Clear();
    mContentsFolder->AppendText(overview);
}

void DialogNewProject::showSelectedFiles()
{
    wxString overview;
    if (mFilePaths.empty())
    {
        overview << sNoFiles;
    }
    else
    {
        if (!mFileAnalyzer)
        {
            mFileAnalyzer = boost::make_shared<model::FileAnalyzer>(mFilePaths,this);
        }
        // else: Dialog canceled or FileAnalyzer already initialized by drag and drop
        overview = getOverviewMessage(mFileAnalyzer);
        if (overview.IsSameAs(sNoFiles))
        {
            mFilePaths.clear();
        }
    }
    mContentsFiles->SetLabel(overview);
    mContentsFiles->Wrap(GetClientSize().GetWidth());
}

wxString DialogNewProject::getOverviewMessage(boost::shared_ptr<model::FileAnalyzer> analyzer) const
{
    wxString result;
    wxString nFiles{ wxString::Format(_("Found %d file(s)"), mFileAnalyzer->getNumberOfMediaFiles()) };

    if (analyzer->getNumberOfMediaFiles() == 0)
    {
        result << sNoFiles;
    }
    else
    {
        // todo make test that touches this for every language!
        result << nFiles << ".";
    }
    return result;
}

}
