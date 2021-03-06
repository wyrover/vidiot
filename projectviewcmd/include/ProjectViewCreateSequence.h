// Copyright 2013-2016 Eric Raijmakers.
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

#pragma once

#include "ProjectViewCommand.h"

namespace cmd {

class ProjectViewCreateSequence
    :   public ProjectViewCommand
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Create a new sequence from a folder. Child folders are not
    /// taken into account. The new sequence is created in a parent folder
    /// of the given folder. That parent folder will always be a non-auto
    /// folder (if a parent is an autofolder, its parent is tried, etc.)
    /// \pre folder must not be the root folder (it must have a parent)
    /// \folder folder to be converted into a sequence
    ProjectViewCreateSequence(const model::FolderPtr& folder);

    /// Create a new sequence in the given folder
    /// \pre folder is not an autofolder
    /// \param folder parent folder to hold the sequence
    /// \param name name of new sequence
    /// \param nodes list of media files for sequence
    ProjectViewCreateSequence(const model::FolderPtr& folder, const wxString& name, const model::NodePtrs& nodes = model::NodePtrs());

    virtual ~ProjectViewCreateSequence();

    //////////////////////////////////////////////////////////////////////////
    // WXCOMMAND
    //////////////////////////////////////////////////////////////////////////

    bool Do() override;
    bool Undo() override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    /// \return the created sequence AFTER it is created.
    model::SequencePtr getSequence() const;

    wxString getName() const;
    model::FolderPtr getParent() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxString mName;
    model::FolderPtr mParent;
    model::SequencePtr mSequence;
    model::NodePtrs mNodes;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Find the first parent (starting from the direct parent, going upwards
    /// in the hierachy) that is not an autofolder.
    /// \pre node has a parent
    /// \param node child node for which such a parent is searched
    /// \return first parent that can be used for adding assets
    model::FolderPtr findFirstNonAutoFolderParent(const model::NodePtr& node) const;
};

} // namespace
