// Copyright 2013-2015 Eric Raijmakers.
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

#include "ProjectViewCreateSequence.h"

#include "AutoFolder.h"
#include "Folder.h"
#include "Sequence.h"
#include "TimelinesView.h"
#include "TrackCreator.h"
#include "Window.h"

namespace cmd {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ProjectViewCreateSequence::ProjectViewCreateSequence(const model::FolderPtr& folder)
    :   ProjectViewCommand()
    ,   mName(folder->getName())
    ,   mParent(findFirstNonAutoFolderParent(folder))
    ,   mSequence()
    ,   mNodes(folder->getChildren())
{
    VAR_INFO(this)(mParent)(folder);
    ASSERT(mParent); // Parent folder must exist

    if (folder->isA<model::AutoFolder>())
    {
        // getName for an autofolder sometimes returns a full path instead of just
        // 'the last path part'. This happens particularly when the auto folder is the
        // topmost autofolder in the hierarchy.
        mName = util::path::toName(boost::dynamic_pointer_cast<model::AutoFolder>(folder)->getPath());
    }
    mCommandName = _("Create sequence from folder ") + mName;
}

ProjectViewCreateSequence::ProjectViewCreateSequence(const model::FolderPtr& folder, const wxString& name, const model::NodePtrs& nodes)
    :   ProjectViewCommand()
    ,   mName(name)
    ,   mParent(folder)
    ,   mSequence()
    ,   mNodes(nodes)
{
    VAR_INFO(mParent)(name);
    ASSERT(mParent); // Parent folder must exist
    ASSERT(!folder->isA<model::AutoFolder>());
    mCommandName = _("Create sequence ") + mName;
}

ProjectViewCreateSequence::~ProjectViewCreateSequence()
{
}

//////////////////////////////////////////////////////////////////////////
// WXCOMMAND
//////////////////////////////////////////////////////////////////////////

bool ProjectViewCreateSequence::Do()
{
    VAR_INFO(this);
    if (!mSequence)
    {
        mSequence = boost::make_shared<model::Sequence>(mName);

        TrackCreator c(mNodes);
        model::Tracks vt = mSequence->getVideoTracks();
        model::Tracks at = mSequence->getAudioTracks();
        mSequence->addVideoTracks({ c.getVideoTrack() });
        mSequence->addAudioTracks({ c.getAudioTrack() });
        mSequence->removeVideoTracks(vt);
        mSequence->removeAudioTracks(at);
    }

    mParent->addChild(mSequence);
    gui::Window::get().getTimeLines().Open(mSequence);

    // May never return false, since then the command is deleted by the command processor. See the use of this object in projectview.
    return true;
}

bool ProjectViewCreateSequence::Undo()
{
    VAR_INFO(this);
    mParent->removeChild(mSequence);
    return true;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

model::SequencePtr ProjectViewCreateSequence::getSequence() const
{
    return mSequence;
}

wxString ProjectViewCreateSequence::getName() const
{
    return mName;
}

model::FolderPtr ProjectViewCreateSequence::getParent() const
{
    return mParent;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

model::FolderPtr ProjectViewCreateSequence::findFirstNonAutoFolderParent(const model::NodePtr& node) const
{
    model::NodePtr parent = node->getParent();
    ASSERT(parent)(node);

    while (parent->isA<model::AutoFolder>())
    {
        parent = parent->getParent();

    }
    model::FolderPtr folder = boost::dynamic_pointer_cast<model::Folder>(parent);
    ASSERT(folder)(parent);
    return folder;
}

} // namespace
