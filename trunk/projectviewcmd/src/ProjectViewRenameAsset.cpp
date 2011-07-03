#include "ProjectViewRenameAsset.h"

#include "UtilLog.h"

namespace command {

ProjectViewRenameAsset::ProjectViewRenameAsset(model::NodePtr node, wxString assetName)
:   ProjectViewCommand()
,   mNode(node)
,   mNewName(assetName)
,   mOldName(node->getName())
{
    VAR_INFO(this)(mNode)(mNewName);
    mCommandName = _("Rename ") + _("\"") + mOldName + _("\" to \"") + mNewName + _("\"");
}

ProjectViewRenameAsset::~ProjectViewRenameAsset()
{
}

bool ProjectViewRenameAsset::Do()
{
    VAR_INFO(this);
    mNode->setName(mNewName);
    return true;
}

bool ProjectViewRenameAsset::Undo()
{
    VAR_INFO(this);
    mNode->setName(mOldName);
    return true;
}

} // namespace