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