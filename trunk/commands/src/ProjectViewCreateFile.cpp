#include "ProjectCommandCreateFile.h"
#include "UtilLog.h"
#include "UtilLogGeneric.h"
#include "UtilLogBoost.h"
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include "File.h"

ProjectCommandCreateFile::ProjectCommandCreateFile(model::FolderPtr parent, std::vector<boost::filesystem::path> paths)
:   ProjectCommand()
,   mParent(parent)
,   mPaths(paths)
,   mChildren()
{
    VAR_INFO(this)(mParent)(mPaths);
    ASSERT(paths.size() > 0);
    if (paths.size() == 1)
    {
        mCommandName = _("Add file")        + _(" \"")   + paths[0].leaf()  + _("\"");
    }
    else
    {
        mCommandName = _("Add files");
    }
}

ProjectCommandCreateFile::~ProjectCommandCreateFile()
{
}

bool ProjectCommandCreateFile::Do()
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
        mParent->addChild(child);//
    }
    return true;
}

bool ProjectCommandCreateFile::Undo()
{
    VAR_INFO(this);
    BOOST_FOREACH(model::ProjectViewPtr child, mChildren)
    {
        mParent->removeChild(child);
    }
    return true;
}
