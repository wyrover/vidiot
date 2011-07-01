#include "ProjectViewCreateFile.h"

#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
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
    ASSERT(paths.size() > 0);
    if (paths.size() == 1)
    {
        mCommandName = _("Add file")        + _(" \"")   + paths[0].GetFullName()  + _("\"");
    }
    else
    {
        mCommandName = _("Add files");
    }
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
            mChildren.push_back(file);
        }
    }
    BOOST_FOREACH(model::FilePtr child, mChildren)
    {
        mParent->addChild(boost::static_pointer_cast<model::AProjectViewNode>(child));//
    }
    return true;
}

bool ProjectViewCreateFile::Undo()
{
    VAR_INFO(this);
    BOOST_FOREACH(model::FilePtr child, mChildren)
    {
        mParent->removeChild(boost::static_pointer_cast<model::AProjectViewNode>(child));
    }
    return true;
}

} // namespace