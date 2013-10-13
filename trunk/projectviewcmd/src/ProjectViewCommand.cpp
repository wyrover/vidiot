// Copyright 2013 Eric Raijmakers.
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

#include "ProjectViewCommand.h"

#include "Node.h"

#include "UtilLog.h"

namespace command {

ProjectViewCommand::ProjectViewCommand()
:   RootCommand()
{
}

ProjectViewCommand::~ProjectViewCommand()
{
}

// static
ParentAndChildPairs ProjectViewCommand::makeParentAndChildPairs(model::NodePtrs children)
{
    ASSERT_MORE_THAN_ZERO(children.size());

    model::NodePtrs prunedlist = ProjectViewCommand::prune(children);
    ParentAndChildPairs pairs;
    for ( model::NodePtr child : prunedlist )
    {
        pairs.push_back(std::make_pair(child->getParent(),child));
    }
    return pairs;
}

bool isDescendantOf(model::NodePtr descendant, model::NodePtr ascendant)
{
    model::NodePtr directparent = descendant->getParent();
    if (!directparent)
    {
        // Orphan: root node, or node was deleted/cut.
        return false;
    }
    else if (directparent == ascendant)
    {
        return true;
    }
    else
    {
        return isDescendantOf(directparent, ascendant);
    }
}

// static
model::NodePtrs ProjectViewCommand::prune(model::NodePtrs children)
{
    ASSERT_MORE_THAN_ZERO(children.size());

    model::NodePtrs newlist;
    for ( model::NodePtr child : children )
    {
        bool ascendantFound = false;
        for ( model::NodePtr possibleParent : children )
        {
            if (isDescendantOf(child,possibleParent))
            {
                ascendantFound = true;
                break;
            }
        }

        if (!ascendantFound)
        {
            // If an ascendant of this node is also deleted/moved, then
            // a move of this node is ignored. Its position
            // within that ascendant child trees is kept intact, only
            // the ascendant will be added/deleted/moved/etc.
            newlist.push_back(child);
        }
    }
    return newlist;
}

} // namespace