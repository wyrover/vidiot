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

#include "ProjectViewCommand.h"

#include "Dialog.h"
#include "File.h"
#include "Node.h" 

namespace cmd {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ProjectViewCommand::ProjectViewCommand()
    :   RootCommand()
{
}

ProjectViewCommand::~ProjectViewCommand()
{
}

//////////////////////////////////////////////////////////////////////////
// INTERFACE
//////////////////////////////////////////////////////////////////////////

// static
ParentAndChildPairs ProjectViewCommand::makeParentAndChildPairs(const model::NodePtrs& children)
{
    if (children.empty()) { LOG_WARNING; }

    model::NodePtrs prunedlist{ ProjectViewCommand::prune(children) };
    ParentAndChildPairs pairs;
    for ( model::NodePtr child : prunedlist )
    {
        pairs.emplace_back(std::make_pair(child->getParent(),child));
    }
    return pairs;
}

bool isDescendantOf(const model::NodePtr& descendant, const model::NodePtr& ascendant)
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
model::NodePtrs ProjectViewCommand::prune(const model::NodePtrs& children)
{
    if (children.empty()) { LOG_WARNING; }

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
            newlist.emplace_back(child);
        }
    }
    return newlist;
}

bool ProjectViewCommand::addNodes(const ParentAndChildPairs& pairs)
{
    static const wxString sCant = _("Problem");
    for ( ParentAndChildPair p : pairs )
    {
        model::IPathPtr path = boost::dynamic_pointer_cast<model::IPath>(p.second);
        if (path && !path->getPath().Exists())
        {
            gui::Dialog::get().getConfirmation(sCant, wxString::Format(_("%s has been removed from disk."), path->getPath().GetLongPath()));
            return false;
        }
        model::FilePtr file = boost::dynamic_pointer_cast<model::File>(p.second);
        if (file && !file->canBeOpened())
        {
            gui::Dialog::get().getConfirmation(sCant, wxString::Format(_("%s can not be opened."), file->getName()));
            return false;
        }
    }
    for ( ParentAndChildPair p : pairs )
    {
        p.first->addChild(p.second);
    }
    return true;
}

bool ProjectViewCommand::removeNodes(const ParentAndChildPairs& pairs)
{
    for ( ParentAndChildPair p : pairs )
    {
        p.first->removeChild(p.second);
    }
    return true;
}

} // namespace