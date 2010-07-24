#ifndef PROJECT_VIEW_CREATE_SEQUENCE_H
#define PROJECT_VIEW_CREATE_SEQUENCE_H

#include <boost/filesystem/path.hpp>
#include "ProjectViewCommand.h"
#include "Folder.h"
#include "Sequence.h"

namespace command {

class ProjectViewCreateSequence 
    :   public ProjectViewCommand
{
public:
    
    /** Create from autofolder */
    ProjectViewCreateSequence(model::FolderPtr folder);

    /** Create new sequence in folder */
    ProjectViewCreateSequence(model::FolderPtr folder, wxString name);

    ~ProjectViewCreateSequence();
    
    bool Do();
    
    bool Undo();

    /** Returns the created sequence AFTER it is created */
    model::SequencePtr getSequence();

private:
    wxString mName;
    model::FolderPtr mParent;
    model::FolderPtr mInputFolder;
    model::SequencePtr mSequence;
};

} // namespace

#endif // PROJECT_VIEW_CREATE_SEQUENCE_H
