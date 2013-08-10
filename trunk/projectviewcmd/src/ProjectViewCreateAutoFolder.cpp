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

#include "ProjectViewCreateAutoFolder.h"

#include "AutoFolder.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "UtilPath.h"

namespace command {

ProjectViewCreateAutoFolder::ProjectViewCreateAutoFolder(model::FolderPtr parent, wxFileName path)
:   ProjectViewCommand()
,   mParent(parent)
,   mNewAutoFolder()
,   mPath(path)
{
    VAR_INFO(this)(mParent)(mPath);
    mCommandName = _("Add folder")  + _(" \"")   + util::path::toName(mPath)  + _("\"");
}

ProjectViewCreateAutoFolder::~ProjectViewCreateAutoFolder()
{
}

bool ProjectViewCreateAutoFolder::Do()
{
    if (!mNewAutoFolder)
    {
        mNewAutoFolder = boost::make_shared<model::AutoFolder>(mPath);
        mNewAutoFolder->setParent(mParent);
    }
    mParent->addChild(mNewAutoFolder);
    // Update must be done AFTER adding the addition of the folder. Otherwise,
    // double entries are made visible in the ProjectView.
    mNewAutoFolder->check();
    return true;
}

bool ProjectViewCreateAutoFolder::Undo()
{
    mParent->removeChild(mNewAutoFolder);
    return true;
}

} // namespace