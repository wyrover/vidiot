#include "ProjectViewCommand.h"

#include "Node.h"
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
ParentAndChildPairs ProjectViewCommand::makeParentAndChildPairs(model::NodePtrs children)
{
    ASSERT_MORE_THAN_ZERO(children.size());

    model::NodePtrs prunedlist = ProjectViewCommand::prune(children);
    ParentAndChildPairs pairs;
    BOOST_FOREACH( model::NodePtr child, prunedlist )
    {
        pairs.push_back(std::make_pair<model::NodePtr,model::NodePtr>(child->getParent(),child));
    }
    return pairs;
}

bool isDescendantOf(model::NodePtr descendant, model::NodePtr ascendant)
{
    model::NodePtr directparent = descendant->getParent();
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
model::NodePtrs ProjectViewCommand::prune(model::NodePtrs children)
{
    ASSERT_MORE_THAN_ZERO(children.size());

    model::NodePtrs newlist;
    BOOST_FOREACH( model::NodePtr child, children )
    {
        bool ascendantFound = false;
        BOOST_FOREACH( model::NodePtr possibleParent, children )
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