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

#pragma once

#include "ProjectViewCommand.h"
#include "Node.h"

namespace command {

class ProjectViewDeleteAsset : public ProjectViewCommand
{
public:
    ProjectViewDeleteAsset(const model::NodePtrs& nodes);
    virtual ~ProjectViewDeleteAsset();
    bool Do() override;
    bool Undo() override;
private:
    ParentAndChildPairs mPairs;
};

} // namespace
