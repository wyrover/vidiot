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

#include "ProjectViewDeleteAsset.h"

#include "File.h"
#include "UtilLog.h"
#include "UtilLogStl.h"

namespace command {

ProjectViewDeleteAsset::ProjectViewDeleteAsset(const model::NodePtrs& nodes)
:   ProjectViewCommand()
,   mPairs(ProjectViewCommand::makeParentAndChildPairs(nodes))
{
    VAR_INFO(this)(mPairs);
    mCommandName = _("Delete");
}

ProjectViewDeleteAsset::~ProjectViewDeleteAsset()
{
}

bool ProjectViewDeleteAsset::Do()
{
    VAR_INFO(this);
    return removeNodes(mPairs);
}

bool ProjectViewDeleteAsset::Undo()
{
    VAR_INFO(this);
    return addNodes(mPairs);
}

} // namespace