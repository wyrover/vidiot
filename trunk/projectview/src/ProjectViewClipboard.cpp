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

#include "ProjectViewClipboard.h"

#include "ProjectModification.h"
#include "ProjectView.h"
#include "ProjectViewCtrl.h"
#include "ProjectViewDeleteAsset.h"
#include "StatusBar.h"

namespace gui {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ProjectViewClipboard::ProjectViewClipboard(ProjectView& projectView)
    :   mProjectView(projectView)
{
    LOG_INFO;

    gui::Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED, &ProjectViewClipboard::onCutFromMainMenu, this, wxID_CUT);
    gui::Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED, &ProjectViewClipboard::onCopyFromMainMenu, this, wxID_COPY);
    gui::Window::get().Bind(wxEVT_COMMAND_MENU_SELECTED, &ProjectViewClipboard::onPasteFromMainMenu, this, wxID_PASTE);
}

ProjectViewClipboard::~ProjectViewClipboard()
{
    gui::Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED, &ProjectViewClipboard::onCutFromMainMenu, this, wxID_CUT);
    gui::Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED, &ProjectViewClipboard::onCopyFromMainMenu, this, wxID_COPY);
    gui::Window::get().Unbind(wxEVT_COMMAND_MENU_SELECTED, &ProjectViewClipboard::onPasteFromMainMenu, this, wxID_PASTE);
}

//////////////////////////////////////////////////////////////////////////
// MAIN WINDOW EDIT MENU
//////////////////////////////////////////////////////////////////////////

void ProjectViewClipboard::onCutFromMainMenu(wxCommandEvent& event)
{
    bool focus = hasKeyboardFocus();
    event.Skip(!focus);
    if (focus)
    {
        onCut();
    }
}

void ProjectViewClipboard::onCopyFromMainMenu(wxCommandEvent& event)
{
    bool focus = hasKeyboardFocus();
    event.Skip(!focus);
    if (focus)
    {
        onCopy();
    }
}

void ProjectViewClipboard::onPasteFromMainMenu(wxCommandEvent& event)
{
    // only if one node is selected and that node is a folder or no node is selected (root node)
    bool focus = hasKeyboardFocus();
    event.Skip(!focus);
    if (focus)
    {
        onPaste();
    }
}

//////////////////////////////////////////////////////////////////////////
// POPUP MENU
//////////////////////////////////////////////////////////////////////////


void ProjectViewClipboard::onCut()
{
    if (storeSelectionInClipboard())
    {
        model::ProjectModification::submitIfPossible(new cmd::ProjectViewDeleteAsset(mProjectView.getSelection()));
    }
}

void ProjectViewClipboard::onCopy()
{
    storeSelectionInClipboard();
}

void ProjectViewClipboard::onPaste()
{
    pasteFromClipboard();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

bool ProjectViewClipboard::hasKeyboardFocus() const
{
    wxWindow* focused = wxWindow::FindFocus();
    if (focused != 0)
    {
#ifdef _MSC_VER
        return (dynamic_cast<ProjectViewCtrl*>(focused->GetParent()) != 0);
#else
        return (dynamic_cast<ProjectViewCtrl*>(focused) != 0);
#endif
    }
    return false;
}

bool ProjectViewClipboard::storeSelectionInClipboard() const
{
    if (mProjectView.getSelection().empty())
    {
        StatusBar::get().timedInfoText(_("Nothing selected in the project view."));
    }
    else if (mProjectView.selectionContainsRootNode())
    {
        StatusBar::get().timedInfoText(_("Project (topmost item) cannot be stored in clipboard."));
    }
    else
    {
        // For nodes in an autofolder, do not copy the nodes if the autofolder is also selected.
        model::NodePtrs selection = cmd::ProjectViewCommand::prune(mProjectView.getSelection());
        if (wxTheClipboard->Open())
        {
            wxTheClipboard->SetData(new ProjectViewDataObject(selection));
            wxTheClipboard->Close();
            return true;
        }
    }
    return false;
}

void ProjectViewClipboard::pasteFromClipboard()
{
    model::FolderPtr target = mProjectView.getSelectedContainerOrRoot();
    if (!target)
    {
        StatusBar::get().timedInfoText(_("Wrong node selected for pasting in project tree."));
        return;
    }
    if (wxTheClipboard->Open())
    {
        if (wxTheClipboard->IsSupported(ProjectViewDataObject::sFormat))
        {
            ProjectViewDataObject data;
            wxTheClipboard->GetData(data);
            wxTheClipboard->Close();
            if (data.getNodes().size() > 0)
            {
                model::NodePtrs currentNodes = model::Project::get().getRoot()->getAllDescendants();
                for (model::NodePtr node : data.getNodes())
                {
                    if (std::find(currentNodes.begin(), currentNodes.end(), node) != currentNodes.end())
                    {
                        StatusBar::get().timedInfoText(_("Can't paste item in tree twice."));
                        return;
                    }
                    if (node->isA<model::File>())
                    {
                        model::FilePtr file = boost::dynamic_pointer_cast<model::File>(node);
                        if (!wxFileExists(file->getPath().GetFullPath()))
                        {
                            StatusBar::get().timedInfoText(_("File was removed from disk. Cannot paste."));
                            return;
                        }
                    }
                    else if (node->isA<model::AutoFolder>())
                    {
                        model::AutoFolderPtr folder = boost::dynamic_pointer_cast<model::AutoFolder>(node);
                        if (!wxDirExists(folder->getPath().GetFullPath()))
                        {
                            StatusBar::get().timedInfoText(_("Folder was removed from disk. Cannot paste."));
                            return;
                        }
                    }
                    if (mProjectView.findConflictingName(target, node->getName(), NODETYPE_ANY))
                    {
                        return;
                    }
                }
                model::ProjectModification::submitIfPossible(new cmd::ProjectViewAddAsset(target, data.getNodes()));
            }
        }
        else if (wxTheClipboard->IsSupported(wxDataFormat(wxDF_FILENAME)))
        {
            wxFileDataObject data;
            wxTheClipboard->GetData(data);
            wxTheClipboard->Close();
            bool files = false;
            bool dirs = false;
            for (wxString filename : data.GetFilenames())
            {
                files = files || (wxFileExists(filename));
                dirs = dirs || (wxDirExists(filename));
            }
            if (files && dirs)
            {
                StatusBar::get().timedInfoText(_("Cannot paste files and folders together. Only paste files or only paste folders."));
                return;
            }

            if (files || dirs) // This check ensures the existence of the files/folders
            {
                model::NodePtrs nodes;
                for (wxString filename : data.GetFilenames())
                {
                    if (files)
                    {
                        model::FilePtr file = boost::make_shared<model::File>(filename);
                        if (!file->canBeOpened())
                        {
                            StatusBar::get().timedInfoText(_("File " + file->getName() + " is not supported."));
                            return;
                        }
                        nodes.push_back(file);
                    }
                    else
                    {
                        nodes.push_back(boost::make_shared<model::AutoFolder>(filename));
                    }
                }
                if (!model::ProjectModification::submitIfPossible(new cmd::ProjectViewAddAsset(target, nodes)))
                {
                    StatusBar::get().timedInfoText(_("No supported files in clipboard."));
                }
            }
        }
        else
        {
            wxTheClipboard->Close();
        }
    }
}

} // namespace
