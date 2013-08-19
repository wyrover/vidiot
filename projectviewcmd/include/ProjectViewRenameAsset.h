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

#ifndef PROJECT_VIEW_RENAME_ASSET_H
#define PROJECT_VIEW_RENAME_ASSET_H

#include "ProjectViewCommand.h"
#include "Node.h"

namespace command {

class ProjectViewRenameAsset : public ProjectViewCommand
{
public:
    ProjectViewRenameAsset(model::NodePtr node, wxString assetName);
    virtual ~ProjectViewRenameAsset();
    bool Do() override;
    bool Undo() override;
private:
    model::NodePtr mNode;
    wxString mNewName;
    wxString mOldName;
};

} // namespace

#endif // PROJECT_VIEW_RENAME_ASSET_H
