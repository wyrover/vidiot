#ifndef PROJECT_VIEW_COMMAND_H
#define PROJECT_VIEW_COMMAND_H

#include "RootCommand.h"

namespace command {

class ProjectViewCommand
    :   public RootCommand
{
public:

    ProjectViewCommand();
    ~ProjectViewCommand();

protected:

    /// This methods serves two purposes:
    /// - Prune the list of selected nodes (See ProjectViewCommand::prune)
    /// - Convert the resulting list of pruning to a list of parent and children
    ///   pairs (the currently associated parent with each node is stored).
    static ParentAndChildPairs makeParentAndChildPairs(model::NodePtrs children);

    /// Strip any nodes for which an ascendant (direct or indirect parent)
    /// is also in the list of nodes. Example: when selecting both a parent
    /// node AND a child node of this parent, moving these two should
    /// only move the actual parent, and keep the parent child relation
    /// intact.
    static model::NodePtrs prune(model::NodePtrs children);
};

} // namespace

#endif // PROJECT_VIEW_COMMAND_H