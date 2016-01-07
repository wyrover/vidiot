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

#include "ProjectViewRenameAsset.h"

namespace cmd {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ProjectViewRenameAsset::ProjectViewRenameAsset(const model::NodePtr& node, const wxString& assetName)
:   ProjectViewCommand()
,   mNode(node)
,   mNewName(assetName)
,   mOldName(node->getName())
{
    VAR_INFO(this)(mNode)(mNewName);
    mCommandName = wxString::Format(_("Rename %1$s to %2$s"), mOldName, mNewName);
}

ProjectViewRenameAsset::~ProjectViewRenameAsset()
{
}

//////////////////////////////////////////////////////////////////////////
// WXCOMMAND
//////////////////////////////////////////////////////////////////////////

bool ProjectViewRenameAsset::Do()
{
    VAR_INFO(this);
    mNode->setName(mNewName);
    return true;
}

bool ProjectViewRenameAsset::Undo()
{
    VAR_INFO(this);
    mNode->setName(mOldName);
    return true;
}

} // namespace