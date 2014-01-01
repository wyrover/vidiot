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

#include "ProjectViewDeleteUnusedFiles.h"

#include "AutoFolder.h"
#include "Clip.h"
#include "ClipInterval.h"
#include "Dialog.h"
#include "File.h"
#include "Project.h"
#include "Sequence.h"
#include "Track.h"
#include "StatusBar.h"
#include "UtilLog.h"
#include "UtilPath.h"
#include "UtilRecycle.h"

namespace command {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ProjectViewDeleteUnusedFiles::ProjectViewDeleteUnusedFiles(model::AutoFolderPtr folder)
:   ProjectViewCommand()
,   mFolder(folder)
,   mTotalSize(wxInvalidSize)
{
    VAR_INFO(this)(folder);
    mCommandName = _("Delete unused files from folder " + folder->getName());
    ASSERT(folder);

    for ( model::NodePtr child : folder->getAllDescendants() )
    {
        model::FilePtr file = boost::dynamic_pointer_cast<model::File>(child);
        if (file && !findInProject(file))
        {
            mFiles.push_back(file);
        }
    }

    for ( model::FilePtr file : mFiles )
    {
        wxULongLong fileSize = file->getPath().GetSize();
        if (fileSize != wxInvalidSize)
        {
            if (mTotalSize != wxInvalidSize)
            {
                mTotalSize += fileSize;
            }
        }
        else
        {
            mTotalSize = wxInvalidSize;
        }
    }
}

ProjectViewDeleteUnusedFiles::~ProjectViewDeleteUnusedFiles()
{
}

bool ProjectViewDeleteUnusedFiles::Do()
{
    VAR_INFO(this);
    return true;
}

bool ProjectViewDeleteUnusedFiles::Undo()
{
    VAR_INFO(this);
    return true;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

void ProjectViewDeleteUnusedFiles::recycleFiles()
{
    wxString size;
    if (mTotalSize != wxInvalidSize)
    {
        size = "\n\n" + _("Disk space freed: ") + wxFileName::GetHumanReadableSize(mTotalSize);
    }

    std::list<wxString> fileNames;
    for ( model::FilePtr file : mFiles )
    {
        fileNames.push_back(util::path::normalize(file->getPath()).GetFullPath());
    }

    std::list<wxString> deleted = gui::Dialog::get().getStringsSelection(
        _("Remove unused files"),
        _("The following files are not used in the project.\nPress OK to move these to the recycle bin.") + size,
        fileNames);

    wxString message;
    for ( wxString file : deleted )
    {
        bool ok = util::path::recycle(file);

        if (!ok)
        {
            message = _("Failed to move") + "'" + file + "'" + _(" to the recycle bin.\nThe following files have already been moved:\n") + message + _("\n\nAborting deletion.");
            gui::Dialog::get().getConfirmation(_("Move to recycle bin failed"), message);
            return;
        }
        else
        {
            message += file + "\n";
        }
    }
    switch (deleted.size())
    {
    case 0:
        gui::StatusBar::get().timedInfoText(_("No files moved to recycle bin."));
        break;
    case 1:
        gui::StatusBar::get().timedInfoText(_("File moved to recycle bin."));
        break;
    default:
        gui::StatusBar::get().timedInfoText(wxString::Format("%d",deleted.size()) + _(" files moved to recycle bin."));
        break;
    }
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

bool ProjectViewDeleteUnusedFiles::findInProject(model::FilePtr file)
{
    model::NodePtr root = model::Project::get().getRoot();
    wxString path = util::path::normalize(file->getPath()).GetFullPath();
    for ( model::NodePtr node : root->getAllDescendants() )
    {
        model::SequencePtr sequence = boost::dynamic_pointer_cast<model::Sequence>(node);
        if (sequence)
        {
            for ( model::TrackPtr track : sequence->getTracks() )
            {
                for ( model::IClipPtr clip : track->getClips() )
                {
                    if (clip->isA<model::ClipInterval>())
                    {
                        if (clip->getFile())
                        {
                            if (util::path::normalize(clip->getFile()->getPath()).GetFullPath().IsSameAs(path))
                            {
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

} // namespace