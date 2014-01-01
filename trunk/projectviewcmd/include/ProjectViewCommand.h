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

#ifndef PROJECT_VIEW_COMMAND_H
#define PROJECT_VIEW_COMMAND_H

#include "RootCommand.h"

namespace command {

class ProjectViewCommand
    :   public RootCommand
{
public:

    ProjectViewCommand();
    virtual ~ProjectViewCommand();

protected:

    /// This methods serves two purposes:
    /// - Prune the list of selected nodes (See ProjectViewCommand::prune)
    /// - Convert the resulting list of pruning to a list of parent and children
    ///   pairs (the currently associated parent with each node is stored).
    static ParentAndChildPairs makeParentAndChildPairs(model::NodePtrs children);

    /// Strip any nodes for which an ascendant (direct or indirect parent)
    /// is also in the list of nodes. Example: when selecting both a parent
    /// node AND a child node of this parent, moving these two should
    /// only move the actual parent, and keep the parent child relation
    /// intact.
    static model::NodePtrs prune(model::NodePtrs children);
};

} // namespace

#endif // PROJECT_VIEW_COMMAND_H