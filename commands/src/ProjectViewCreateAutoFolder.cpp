#include "ProjectCommandCreateAutoFolder.h"
#include "UtilLog.h"
#include "UtilLogBoost.h"
#include <boost/make_shared.hpp>

ProjectCommandCreateAutoFolder::ProjectCommandCreateAutoFolder(model::FolderPtr parent, boost::filesystem::path path)
:   ProjectCommand()
,   mParent(parent)
,   mNewAutoFolder()
,   mPath(path)
{
    VAR_INFO(this)(mParent)(mPath);
    mCommandName = _("Add autofolder")  + _(" \"")   + mPath.leaf()  + _("\"");
}

ProjectCommandCreateAutoFolder::~ProjectCommandCreateAutoFolder()
{
}

bool ProjectCommandCreateAutoFolder::Do()
{
    if (!mNewAutoFolder)
    {
        mNewAutoFolder = boost::make_shared<model::AutoFolder>(mPath);
        mNewAutoFolder->setParent(mParent);
    }
    mParent->addChild(mNewAutoFolder);
    return true;
}

bool ProjectCommandCreateAutoFolder::Undo()
{
    mParent->removeChild(mNewAutoFolder);
    return true;
}
