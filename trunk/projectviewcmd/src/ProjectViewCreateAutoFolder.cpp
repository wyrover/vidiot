#include "ProjectViewCreateAutoFolder.h"

#include "AutoFolder.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "UtilPath.h"

namespace command {

ProjectViewCreateAutoFolder::ProjectViewCreateAutoFolder(model::FolderPtr parent, wxFileName path)
:   ProjectViewCommand()
,   mParent(parent)
,   mNewAutoFolder()
,   mPath(path)
{
    VAR_INFO(this)(mParent)(mPath);
    mCommandName = _("Add folder")  + _(" \"")   + util::path::toName(mPath)  + _("\"");
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
    // Update must be done AFTER adding the addition of the folder. Otherwise,
    // double entries are made visible in the ProjectView.
    mNewAutoFolder->update(true);
    return true;
}

bool ProjectViewCreateAutoFolder::Undo()
{
    mParent->removeChild(mNewAutoFolder);
    return true;
}

} // namespace