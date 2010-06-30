#ifndef PROJECT_COMMAND_H
#define PROJECT_COMMAND_H

#include <vector>
#include <wx/cmdproc.h>
#include <wx/intl.h>
#include "AProjectViewNode.h"

typedef std::pair<model::ProjectViewPtr ,model::ProjectViewPtr>  ParentAndChildPair;
typedef std::vector<ParentAndChildPair>  ParentAndChildPairs;

namespace command {

class ProjectCommand : public wxCommand
{
public:

    ProjectCommand();
    ~ProjectCommand();
    wxString GetName() const;

protected:

    /**
    * This methods serves two purposes:
    * - Prune the list of selected nodes (See ProjectCommand::prune)
    * - Convert the resulting list of pruning to a list of parent and children
    *   pairs (the currently associated parent with each node is stored).
    */
    static ParentAndChildPairs makeParentAndChildPairs(model::ProjectViewPtrs children);

    /**
    * Strip any nodes for which an ascendant (direct or indirect parent)
    * is also in the list of nodes. Example: when selecting both a parent
    * node AND a child node of this parent, moving these two should 
    * only move the actual parent, and keep the parent child relation
    * intact.
    */
    static model::ProjectViewPtrs ProjectCommand::prune(model::ProjectViewPtrs children);

    wxString mCommandName;
};

} // namespace

#endif // PROJECT_COMMAND_H
