#ifndef PROJECT_VIEW_CREATE_SEQUENCE_H
#define PROJECT_VIEW_CREATE_SEQUENCE_H

#include <boost/filesystem/path.hpp>
#include "ProjectViewCommand.h"

namespace model {
class Folder;
typedef boost::shared_ptr<Folder> FolderPtr;
class Sequence;
typedef boost::shared_ptr<Sequence> SequencePtr;
}

namespace command {

class ProjectViewCreateSequence 
    :   public ProjectViewCommand
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    /// Create a new sequence from a folder. Child folders are not 
    /// taken into account. The new sequence is created in a parent folder
    /// of the given folder. That parent folder will always be a non-auto
    /// folder (if a parent is an autofolder, its parent is tried, etc.)
    /// \pre folder must not be the root folder (it must have a parent)
    /// \folder folder to be converted into a sequence
    ProjectViewCreateSequence(model::FolderPtr folder);

    /// Create a new sequence in the given folder
    /// \pre folder is not an autofolder
    /// \param folder parent folder to hold the sequence
    /// \name name of new sequence
    ProjectViewCreateSequence(model::FolderPtr folder, wxString name);

    ~ProjectViewCreateSequence();

    //////////////////////////////////////////////////////////////////////////
    // WXCOMMAND
    //////////////////////////////////////////////////////////////////////////

    bool Do();
    bool Undo();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    /// \return the created sequence AFTER it is created.
    model::SequencePtr getSequence() const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxString mName;
    model::FolderPtr mParent;
    model::FolderPtr mInputFolder;
    model::SequencePtr mSequence;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    /// Find the first parent (starting from the direct parent, going upwards
    /// in the hierachy) that is not an autofolder.
    /// \pre node has a parent
    /// \param node child node for which such a parent is searched
    /// \return first parent that can be used for adding assets
    model::FolderPtr findFirstNonAutoFolderParent(model::ProjectViewPtr node) const;
};

} // namespace

#endif // PROJECT_VIEW_CREATE_SEQUENCE_H
