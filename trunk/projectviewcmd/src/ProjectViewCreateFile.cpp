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

#include "ProjectViewCreateFile.h"

#include "File.h"
#include "Folder.h"

#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "UtilLogStl.h"

namespace command {

ProjectViewCreateFile::ProjectViewCreateFile(model::FolderPtr parent, std::vector<wxFileName> paths)
:   ProjectViewCommand()
,   mParent(parent)
,   mPaths(paths)
,   mChildren()
{
    VAR_INFO(this)(mParent)(mPaths);
    ASSERT_MORE_THAN_ZERO(paths.size());
    mCommandName = _("Add files");
}

ProjectViewCreateFile::~ProjectViewCreateFile()
{
}

bool ProjectViewCreateFile::Do()
{
    VAR_INFO(this);
    if (mChildren.size() == 0)
    {
        BOOST_FOREACH( wxFileName path, mPaths )
        {
            model::FilePtr file = boost::make_shared<model::File>(path);
            if (file->canBeOpened())
            {
                mChildren.push_back(file);
            }
        }
        if (mChildren.size() == 1)
        {
            mCommandName = _("Add file")        + _(" \"")   + mChildren.front()->getDescription()  + _("\"");
        }
    }
    BOOST_FOREACH(model::FilePtr child, mChildren)
    {
        mParent->addChild(boost::static_pointer_cast<model::Node>(child));
    }
    return true;
}

bool ProjectViewCreateFile::Undo()
{
    VAR_INFO(this);
    BOOST_FOREACH(model::FilePtr child, mChildren)
    {
        mParent->removeChild(boost::static_pointer_cast<model::Node>(child));
    }
    return true;
}

} // namespace