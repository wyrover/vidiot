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

#include "ProjectViewAddAsset.h"
#include "UtilLog.h"
#include "UtilLogStl.h"

namespace command {

ProjectViewAddAsset::ProjectViewAddAsset(model::NodePtr parent, model::NodePtrs nodes)
:   ProjectViewCommand()
,   mParent(parent)
,   mChildren(ProjectViewCommand::prune(nodes))
{
    VAR_INFO(this)(mParent)(mChildren);
    mCommandName = _("Insert assets" );
}

ProjectViewAddAsset::~ProjectViewAddAsset()
{
}

bool ProjectViewAddAsset::Do()
{
    VAR_INFO(this);
    BOOST_FOREACH(model::NodePtr child, mChildren)
    {
        mParent->addChild(child);
    }
    return true;
}

bool ProjectViewAddAsset::Undo()
{
    VAR_INFO(this);
    BOOST_FOREACH(model::NodePtr child, mChildren)
    {
        mParent->removeChild(child);
    }
    return true;
}

} // namespace