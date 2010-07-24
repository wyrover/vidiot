#include "ProjectViewDeleteAsset.h"

#include <boost/foreach.hpp>
#include "UtilLog.h"
#include "UtilLogGeneric.h"

namespace command {

ProjectViewDeleteAsset::ProjectViewDeleteAsset(model::ProjectViewPtrs nodes)
:   ProjectViewCommand()
,   mPairs(ProjectViewCommand::makeParentAndChildPairs(nodes))
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

ProjectViewDeleteAsset::~ProjectViewDeleteAsset()
{
    mPairs.clear();
}

/** /todo handle all cross references to this node */
bool ProjectViewDeleteAsset::Do()
{
    VAR_INFO(this);
    BOOST_FOREACH( ParentAndChildPair p, mPairs )
    {
        p.first->removeChild(p.second);
    }
    return true;
}

bool ProjectViewDeleteAsset::Undo()
{
    VAR_INFO(this);
    BOOST_FOREACH( ParentAndChildPair p, mPairs )
    {
        p.first->addChild(p.second);
    }
    return true;
}

} // namespace