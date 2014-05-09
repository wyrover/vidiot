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

#include "DropFilesFromFileSystem.h"

#include "AudioFile.h"
#include "AutoFolder.h"
#include "Dialog.h"
#include "File.h"
#include "Project.h"
#include "ProjectModification.h"
#include "ProjectView.h"
#include "ProjectViewAddAsset.h"
#include "ProjectViewCreateSequence.h"
#include "Properties.h"
#include "StatusBar.h"
#include "UtilEnum.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilLogWxwidgets.h"
#include "UtilPath.h"
#include "VideoFile.h"
#include "Window.h"

namespace command {

void DropFilesFromFileSystem(std::list<wxString> filenames)
{
    VAR_INFO(filenames);

    // Create sorted list of dropped folders and files.
    std::list<wxFileName> files;
    for (wxString path : filenames)
    {
        files.push_back(::util::path::toFileName(path));
    }
    files.sort([](const wxFileName& file1, const wxFileName& file2) { return file1.GetFullPath() < file2.GetFullPath(); });

    // Determine some 'statistics'
    bool autocreate = !gui::Window::get().isProjectOpened();
    int nFolders = std::count_if(files.begin(),files.end(), [](wxFileName file) -> bool { return file.IsDir(); });
    int nFiles = std::count_if(files.begin(),files.end(), [](wxFileName file) -> bool { return !file.IsDir(); });
    bool oneFolderDropped = 
        (nFolders == 1 && nFiles == 0 && 
        files.front().GetDirCount() > 0 &&  // Not a drive root
        !::util::path::hasSubDirectories(files.front()));
    bool onlyFilesDropped =
        (nFolders == 0 && nFiles > 1);

    // Create project if required.
    if (autocreate)
    {
        ::gui::Window::get().openPropertiesUponProjectCreation(false); // Avoid properties popup
        ::gui::Window::get().GetDocumentManager()->CreateNewDocument();
        ::gui::Window::get().openPropertiesUponProjectCreation(true);

        if (oneFolderDropped)
        {
            model::Project::get().SetTitle(""); // Make empty, to take the docname from the filename
            model::Project::get().SetFilename(files.front().GetDirs().Last() + ".vid",true);
        }
    }

    // Create list of nodes, check if files can be opened, and determine the frame/sample rates of the files.
    ::model::NodePtrs nodes;
    std::set<FrameRate> framerates;
    std::set<int> samplerates;
    for (const wxFileName& filename : files)
    {
        if (filename.IsDir())
        {
            nodes.push_back(boost::make_shared<::model::AutoFolder>(filename));
        }
    };
    for (const wxFileName& filename : files)
    {
        if (!filename.IsDir())
        {
            ::model::FilePtr file = boost::make_shared<::model::File>(filename);
            if (file->canBeOpened())
            {
                nodes.push_back(file);
                if (file->hasVideo())
                {
                    model::VideoFilePtr videofile = boost::make_shared<model::VideoFile>(filename);
                    framerates.insert(videofile->getFrameRate());
                }
                if (file->hasAudio())
                {
                    model::AudioFilePtr audiofile = boost::make_shared<model::AudioFile>(filename);
                    samplerates.insert(audiofile->getSampleRate());
                }
            }
        }
    };

    // Checks
    if (nodes.empty())
    {
        ::gui::Dialog::get().getConfirmation(_("No usable files"), _("Vidiot was unable to import any of the dropped files."));
        return;
    }
    for (model::NodePtr node : nodes)
    {
        if (::gui::ProjectView::get().findConflictingName(::model::Project::get().getRoot(), node->getName(), node->isA<model::Folder>() ? ::gui::NODETYPE_FOLDER : ::gui::NODETYPE_FILE)) 
        { 
            return; 
        }
    }

    // Add nodes to the project view
    ::model::FolderPtr root = ::model::Project::get().getRoot();
    model::ProjectModification::submit(new ProjectViewAddAsset(root, nodes)); 
    for (::model::NodePtr node : nodes)
    {
        node->check(true); // Update any added autofolders
    }
    gui::StatusBar::get().hideProgressBar();
    gui::StatusBar::get().setProcessingText("");

    if (autocreate)
    {
        if (framerates.size() == 1 && samplerates.size() == 1)
        {
            model::Properties::get().setFrameRate(*(framerates.begin()));
            model::Properties::get().setAudioFrameRate(*(samplerates.begin()));

            if (oneFolderDropped)
            {
                // Create sequence of this folder
                ::model::AutoFolderPtr folder = boost::dynamic_pointer_cast<::model::AutoFolder>(nodes.front());
                model::ProjectModification::submit(new command::ProjectViewCreateSequence(folder));
            }
            else if (onlyFilesDropped && nodes.size() > 0) // Check for nodes is done to ensure that there are files that can be opened.
            {
                // Create sequence of all given files
                wxString sequenceName = _("Sequence");
                model::ProjectModification::submit(new command::ProjectViewCreateSequence(root, sequenceName, nodes));
            }
        }
    }
}

} // namespace