#include "ProjectViewMoveAsset.h"

#include <boost/foreach.hpp>
#include "UtilLog.h"
#include "UtilLogStl.h"

namespace command {

ProjectViewMoveAsset::ProjectViewMoveAsset(model::ProjectViewPtrs nodes, model::ProjectViewPtr parent)
:   ProjectViewCommand()
,   mNewParent(parent)
,   mPairs(ProjectViewCommand::makeParentAndChildPairs(nodes))
{
    VAR_INFO(this)(mNewParent)(mPairs);
    if (nodes.size() == 1)
    {
        mCommandName = _("Move ") + _("\"") + (*(nodes.begin()))->getName() + _("\"");
    }
    else
    {
        mCommandName = _("Move assets"); 
    }
}

ProjectViewMoveAsset::~ProjectViewMoveAsset()
{
}

bool ProjectViewMoveAsset::Do()
{
    VAR_INFO(this);
    BOOST_FOREACH( ParentAndChildPair p, mPairs )
    {
        // Ignore nodes that are moved to their original
        // parent.
        if (p.first != mNewParent)
        {
            p.first->removeChild(p.second);
            mNewParent->addChild(p.second);
        }
    }
    return true;
}

bool ProjectViewMoveAsset::Undo()
{
    VAR_INFO(this);
    BOOST_FOREACH( ParentAndChildPair p, mPairs )
    {
        mNewParent->removeChild(p.second);
        p.first->addChild(p.second);
    }
    return true;
}

} // namespace