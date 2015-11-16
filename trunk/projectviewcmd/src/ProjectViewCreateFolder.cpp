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

#include "ProjectViewCreateFolder.h"

#include "UtilLog.h"

namespace command {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ProjectViewCreateFolder::ProjectViewCreateFolder(const model::FolderPtr& parent, const wxString& name)
:   ProjectViewCommand()
,   mParent(parent)
,   mNewFolder()
,   mName(name)
{
    VAR_INFO(this)(mParent)(name);
    mCommandName = wxString::Format(_("Add folder %s"), mName);
}

ProjectViewCreateFolder::~ProjectViewCreateFolder()
{
}

//////////////////////////////////////////////////////////////////////////
// WXCOMMAND
//////////////////////////////////////////////////////////////////////////

bool ProjectViewCreateFolder::Do()
{
    VAR_INFO(this);
    if (!mNewFolder)
    {
        mNewFolder = boost::make_shared<model::Folder>(mName);
        mNewFolder->setParent(mParent);
    }
    mParent->addChild(mNewFolder);
    return true;
}

bool ProjectViewCreateFolder::Undo()
{
    VAR_INFO(this);
    mParent->removeChild(mNewFolder);
    return true;
}

} // namespace