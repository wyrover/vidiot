#include "ProjectViewAddAsset.h"
#include "UtilLog.h"
#include "UtilLogStl.h"

namespace command {

ProjectViewAddAsset::ProjectViewAddAsset(model::NodePtr parent, model::NodePtrs nodes)
:   ProjectViewCommand()
,   mParent(parent)
,   mChildren(ProjectViewCommand::prune(nodes))
{
    VAR_INFO(this)(mParent)(mChildren);
    mCommandName = _("Insert assets" );
}

ProjectViewAddAsset::~ProjectViewAddAsset()
{
}

bool ProjectViewAddAsset::Do()
{
    VAR_INFO(this);
    BOOST_FOREACH(model::NodePtr child, mChildren)
    {
        mParent->addChild(child);
    }
    return true;
}

bool ProjectViewAddAsset::Undo()
{
    VAR_INFO(this);
    BOOST_FOREACH(model::NodePtr child, mChildren)
    {
        mParent->removeChild(child);
    }
    return true;
}

} // namespace