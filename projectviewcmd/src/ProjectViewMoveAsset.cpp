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

#include "ProjectViewMoveAsset.h"

namespace cmd {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ProjectViewMoveAsset::ProjectViewMoveAsset(const model::NodePtrs& nodes, const model::NodePtr& parent)
:   ProjectViewCommand()
,   mPairs(ProjectViewCommand::makeParentAndChildPairs(nodes))
,   mNewParent(parent)
{
    VAR_INFO(this)(mNewParent)(mPairs);
    ASSERT_NONZERO(mNewParent);
    if (nodes.size() == 1)
    {
        mCommandName = wxString::Format(_("Move %s"), (*(nodes.begin()))->getName());
    }
    else
    {
        mCommandName = _("Move");
    }
}

ProjectViewMoveAsset::~ProjectViewMoveAsset()
{
}

//////////////////////////////////////////////////////////////////////////
// WXCOMMAND
//////////////////////////////////////////////////////////////////////////

bool ProjectViewMoveAsset::Do()
{
    VAR_INFO(this);
    for ( ParentAndChildPair p : mPairs )
    {
        ASSERT(p.first);
        // Ignore nodes that are moved to their original
        // parent.
        if (p.first != mNewParent)
        {
            p.first->removeChild(p.second);
            mNewParent->addChild(p.second);
        }
    }
    return true;
}

bool ProjectViewMoveAsset::Undo()
{
    VAR_INFO(this);
    for ( ParentAndChildPair p : mPairs )
    {
        mNewParent->removeChild(p.second);
        p.first->addChild(p.second);
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////
// ROOTCOMMAND
//////////////////////////////////////////////////////////////////////////

bool ProjectViewMoveAsset::isPossible() const
{
    return !mPairs.empty();
}

} // namespace
