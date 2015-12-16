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

#include "ProjectViewCreateAutoFolder.h"

#include "AutoFolder.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "UtilPath.h"

namespace cmd {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

ProjectViewCreateAutoFolder::ProjectViewCreateAutoFolder(const model::FolderPtr& parent, const wxFileName& path)
:   ProjectViewCommand()
,   mPair(boost::dynamic_pointer_cast<model::INode>(parent),model::NodePtr())
,   mPath(path)
{
    VAR_INFO(this)(parent)(path);
    mCommandName = wxString::Format(_("Add folder %s"), util::path::toName(mPath));
}

ProjectViewCreateAutoFolder::~ProjectViewCreateAutoFolder()
{
}

//////////////////////////////////////////////////////////////////////////
// wxCommand
//////////////////////////////////////////////////////////////////////////

bool ProjectViewCreateAutoFolder::Do()
{
    ASSERT_IMPLIES(mPair.second, mPair.second->isA<model::AutoFolder>());
    model::AutoFolderPtr autofolder = boost::dynamic_pointer_cast<model::AutoFolder>(mPair.second);
    if (!autofolder)
    {
        autofolder = boost::make_shared<model::AutoFolder>(mPath);
        autofolder->setParent(mPair.first);
    }
    ASSERT_NONZERO(autofolder);
    mPair.second = autofolder;
    bool ok = addNodes({ mPair });
    if (!ok)
    {
        return false; // Folder missing on disk. Can happen in case of Redo.
    }
    // Update must be done AFTER adding the addition of the folder. Otherwise,
    // double entries are made visible in the ProjectView.
    autofolder->check();
    return true;
}

bool ProjectViewCreateAutoFolder::Undo()
{
    return removeNodes({ mPair });
}

} // namespace