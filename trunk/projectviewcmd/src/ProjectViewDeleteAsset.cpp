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

#include "ProjectViewDeleteAsset.h"

#include "UtilLog.h"
#include "UtilLogStl.h"

namespace command {

ProjectViewDeleteAsset::ProjectViewDeleteAsset(model::NodePtrs nodes)
:   ProjectViewCommand()
,   mPairs(ProjectViewCommand::makeParentAndChildPairs(nodes))
{
    VAR_INFO(this)(mPairs);
    if (nodes.size() == 1)
    {
        mCommandName = _("Delete ") + (*(nodes.begin()))->getName();
    }
    else
    {
        mCommandName = _("Delete assets");
    }
}

ProjectViewDeleteAsset::~ProjectViewDeleteAsset()
{
}

bool ProjectViewDeleteAsset::Do()
{
    VAR_INFO(this);
    BOOST_FOREACH( ParentAndChildPair p, mPairs )
    {
        p.first->removeChild(p.second);
    }
    return true;
}

bool ProjectViewDeleteAsset::Undo()
{
    VAR_INFO(this);
    BOOST_FOREACH( ParentAndChildPair p, mPairs )
    {
        p.first->addChild(p.second);
    }
    return true;
}

} // namespace