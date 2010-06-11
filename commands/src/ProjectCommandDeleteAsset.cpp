#include "ProjectCommandDeleteAsset.h"
#include "AProjectViewNode.h"
#include "UtilLog.h"
#include "UtilLogGeneric.h"
#include <boost/foreach.hpp>

ProjectCommandDeleteAsset::ProjectCommandDeleteAsset(model::ProjectViewPtrs nodes)
:   ProjectCommand()
,   mPairs(ProjectCommand::makeParentAndChildPairs(nodes))
{
    VAR_INFO(this)(mPairs);
    if (nodes.size() == 1)
    {
        mCommandName = _("Delete ") + (*(nodes.begin()))->getName();
    }
    else
    {
        mCommandName = _("Delete assets"); 
    }
}

ProjectCommandDeleteAsset::~ProjectCommandDeleteAsset()
{
    mPairs.clear();
}

/** /todo handle all cross references to this node */
bool ProjectCommandDeleteAsset::Do()
{
    VAR_INFO(this);
    BOOST_FOREACH( ParentAndChildPair p, mPairs )
    {
        p.first->removeChild(p.second);
    }
    return true;
}

bool ProjectCommandDeleteAsset::Undo()
{
    VAR_INFO(this);
    BOOST_FOREACH( ParentAndChildPair p, mPairs )
    {
        p.first->addChild(p.second);
    }
    return true;
}
