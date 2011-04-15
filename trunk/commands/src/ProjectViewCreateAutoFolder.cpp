#include "ProjectViewCreateAutoFolder.h"

#include <boost/make_shared.hpp>
#include "UtilLog.h"
#include "UtilLogBoost.h"

namespace command {

ProjectViewCreateAutoFolder::ProjectViewCreateAutoFolder(model::FolderPtr parent, boost::filesystem::path path)
:   ProjectViewCommand()
,   mParent(parent)
,   mNewAutoFolder()
,   mPath(path)
{
    VAR_INFO(this)(mParent)(mPath);
    mCommandName = _("Add autofolder")  + _(" \"")   + mPath.filename().c_str()  + _("\"");
}

ProjectViewCreateAutoFolder::~ProjectViewCreateAutoFolder()
{
}

bool ProjectViewCreateAutoFolder::Do()
{
    if (!mNewAutoFolder)
    {
        mNewAutoFolder = boost::make_shared<model::AutoFolder>(mPath);
        mNewAutoFolder->setParent(mParent);
    }
    mParent->addChild(mNewAutoFolder);
    return true;
}

bool ProjectViewCreateAutoFolder::Undo()
{
    mParent->removeChild(mNewAutoFolder);
    return true;
}

} // namespace