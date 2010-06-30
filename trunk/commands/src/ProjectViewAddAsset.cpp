#include "ProjectViewAddAsset.h"
#include "UtilLog.h"
#include "UtilLogGeneric.h"
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

namespace command {

ProjectViewAddAsset::ProjectViewAddAsset(model::ProjectViewPtr parent, model::ProjectViewPtrs nodes)
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
    BOOST_FOREACH(model::ProjectViewPtr child, mChildren)
    {
        mParent->addChild(child);
    }
    return true;
}

bool ProjectViewAddAsset::Undo()
{
    VAR_INFO(this);
    BOOST_FOREACH(model::ProjectViewPtr child, mChildren)
    {
        mParent->removeChild(child);
    }
    return true;
}

} // namespace