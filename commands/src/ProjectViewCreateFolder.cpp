#include "ProjectViewCreateFolder.h"
#include "UtilLog.h"
#include <boost/make_shared.hpp>

namespace command {

ProjectViewCreateFolder::ProjectViewCreateFolder(model::FolderPtr parent, wxString name)
:   ProjectViewCommand()
,   mParent(parent)
,   mNewFolder()
,   mName(name)
{
    VAR_INFO(this)(mParent)(name);
    mCommandName = _("Add folder \"") + mName + _("\""); 
}

ProjectViewCreateFolder::~ProjectViewCreateFolder()
{
}

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