#include "ProjectCommandMoveAsset.h"
#include <boost/foreach.hpp>
#include "AProjectViewNode.h"
#include "UtilLog.h"
#include "UtilLogGeneric.h"

ProjectCommandMoveAsset::ProjectCommandMoveAsset(model::ProjectViewPtrs nodes, model::ProjectViewPtr parent)
:   ProjectCommand()
,   mNewParent(parent)
,   mPairs(ProjectCommand::makeParentAndChildPairs(nodes))
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

ProjectCommandMoveAsset::~ProjectCommandMoveAsset()
{
    mNewParent.reset();
    mPairs.clear();
}

bool ProjectCommandMoveAsset::Do()
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

bool ProjectCommandMoveAsset::Undo()
{
    VAR_INFO(this);
    BOOST_FOREACH( ParentAndChildPair p, mPairs )
    {
        mNewParent->removeChild(p.second);
        p.first->addChild(p.second);
    }
    return true;
}
