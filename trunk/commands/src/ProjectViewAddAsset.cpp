#include "ProjectCommandAddAsset.h"
#include "UtilLog.h"
#include "UtilLogGeneric.h"
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>

ProjectCommandAddAsset::ProjectCommandAddAsset(model::ProjectViewPtr parent, model::ProjectViewPtrs nodes)
:   ProjectCommand()
,   mParent(parent)
,   mChildren(ProjectCommand::prune(nodes))
{
    VAR_INFO(this)(mParent)(mChildren);
    mCommandName = _("Insert assets" );
}

ProjectCommandAddAsset::~ProjectCommandAddAsset()
{
}

bool ProjectCommandAddAsset::Do()
{
    VAR_INFO(this);
    BOOST_FOREACH(model::ProjectViewPtr child, mChildren)
    {
        mParent->addChild(child);
    }
    return true;
}

bool ProjectCommandAddAsset::Undo()
{
    VAR_INFO(this);
    BOOST_FOREACH(model::ProjectViewPtr child, mChildren)
    {
        mParent->removeChild(child);
    }
    return true;
}
