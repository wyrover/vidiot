#include "ProjectViewCreateFile.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "UtilLogBoost.h"
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include "File.h"

namespace command {

ProjectViewCreateFile::ProjectViewCreateFile(model::FolderPtr parent, std::vector<boost::filesystem::path> paths)
:   ProjectViewCommand()
,   mParent(parent)
,   mPaths(paths)
,   mChildren()
{
    VAR_INFO(this)(mParent)(mPaths);
    ASSERT(paths.size() > 0);
    if (paths.size() == 1)
    {
        mCommandName = _("Add file")        + _(" \"")   + paths[0].filename().c_str()  + _("\"");
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
        BOOST_FOREACH( boost::filesystem::path path, mPaths )
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