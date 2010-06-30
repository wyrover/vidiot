#include "ProjectViewCommand.h"
#include <boost/foreach.hpp>
#include "AProjectViewNode.h"
#include "UtilLog.h"

namespace command {

ProjectViewCommand::ProjectViewCommand()
:   RootCommand()
{
}

ProjectViewCommand::~ProjectViewCommand()
{
}

// static
ParentAndChildPairs ProjectViewCommand::makeParentAndChildPairs(model::ProjectViewPtrs children)
{
    ASSERT(children.size() > 0);

    model::ProjectViewPtrs prunedlist = ProjectViewCommand::prune(children);
    ParentAndChildPairs pairs;
    BOOST_FOREACH( model::ProjectViewPtr child, prunedlist )
    {
        pairs.push_back(std::make_pair<model::ProjectViewPtr,model::ProjectViewPtr>(child->getParent(),child));
    }
    return pairs;
}

bool isDescendantOf(model::ProjectViewPtr descendant, model::ProjectViewPtr ascendant)
{
    model::ProjectViewPtr directparent = descendant->getParent();
    if (!directparent)
    {
        // Orphan: root node, or node was deleted/cut.
        return false;
    }
    else if (directparent == ascendant)
    {
        return true;
    }
    else
    {
        return isDescendantOf(directparent, ascendant);
    }
}

// static
model::ProjectViewPtrs ProjectViewCommand::prune(model::ProjectViewPtrs children)
{
    ASSERT(children.size() > 0);

    model::ProjectViewPtrs newlist;
    BOOST_FOREACH( model::ProjectViewPtr child, children )
    {
        bool ascendantFound = false;
        BOOST_FOREACH( model::ProjectViewPtr possibleParent, children )
        {
            if (isDescendantOf(child,possibleParent))
            {
                ascendantFound = true;
                break;
            }
        }

        if (!ascendantFound)
        {
            // If an ascendant of this node is also deleted/moved, then
            // a move of this node is ignored. Its position
            // within that ascendant child trees is kept intact, only
            // the ascendant will be added/deleted/moved/etc.
            newlist.push_back(child);
        }
    }
    return newlist;
}

} // namespace