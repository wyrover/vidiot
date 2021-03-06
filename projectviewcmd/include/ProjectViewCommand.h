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

#include "RootCommand.h"

namespace cmd {

class ProjectViewCommand
    :   public RootCommand
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    ProjectViewCommand();
    virtual ~ProjectViewCommand();

    //////////////////////////////////////////////////////////////////////////
    // INTERFACE
    //////////////////////////////////////////////////////////////////////////

    /// This methods serves two purposes:
    /// - Prune the list of selected nodes (See ProjectViewCommand::prune)
    /// - Convert the resulting list of pruning to a list of parent and children
    ///   pairs (the currently associated parent with each node is stored).
    static ParentAndChildPairs makeParentAndChildPairs(const model::NodePtrs& children);

    /// Strip any nodes for which an ascendant (direct or indirect parent)
    /// is also in the list of nodes. Example: when selecting both a parent
    /// node AND a child node of this parent, moving these two should
    /// only move the actual parent, and keep the parent child relation
    /// intact.
    static model::NodePtrs prune(const model::NodePtrs& children);

    /// Add nodes to the project view.
    /// This includes checking for the existence of the to be added
    /// children. If one of the children is not available (removed
    /// from disk or replaced with another file that cannot be opened)
    /// then false is returned, and no changes are made.
    bool addNodes(const ParentAndChildPairs& pairs);

    /// Remove nodes from the project view
    bool removeNodes(const ParentAndChildPairs& pairs);

};

} // namespace
