#include "ProjectCommandCreateFolder.h"
#include "UtilLog.h"
#include <boost/make_shared.hpp>

ProjectCommandCreateFolder::ProjectCommandCreateFolder(model::FolderPtr parent, wxString name)
:   ProjectCommand()
,   mParent(parent)
,   mNewFolder()
,   mName(name)
{
    VAR_INFO(this)(mParent)(name);
    mCommandName = _("Add folder \"") + mName + _("\""); 
}

ProjectCommandCreateFolder::~ProjectCommandCreateFolder()
{
}

bool ProjectCommandCreateFolder::Do()
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

bool ProjectCommandCreateFolder::Undo()
{
    VAR_INFO(this);
    mParent->removeChild(mNewFolder);
    return true;
}
