#include "ProjectCommandRenameAsset.h"
#include "AProjectViewNode.h"
#include "UtilLog.h"

ProjectCommandRenameAsset::ProjectCommandRenameAsset(model::ProjectViewPtr node, wxString assetName)
:   ProjectCommand()
,   mNode(node)
,   mNewName(assetName)
,   mOldName(node->getName())
{
    VAR_INFO(this)(mNode)(mNewName);
    mCommandName = _("Rename ") + _("\"") + mOldName + _("\" to \"") + mNewName + _("\"");
}

ProjectCommandRenameAsset::~ProjectCommandRenameAsset()
{
    mNode.reset();
}

bool ProjectCommandRenameAsset::Do()
{
    VAR_INFO(this);
    mNode->setName(mNewName);
    return true;
}

bool ProjectCommandRenameAsset::Undo()
{
    VAR_INFO(this);
    mNode->setName(mOldName);
    return true;
}
