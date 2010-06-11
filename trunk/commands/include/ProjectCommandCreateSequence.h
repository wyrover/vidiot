#ifndef PROJECT_COMMAND_CREATE_SEQUENCE_H
#define PROJECT_COMMAND_CREATE_SEQUENCE_H

#include <boost/filesystem/path.hpp>
#include "ProjectCommand.h"
#include "Folder.h"
#include "Sequence.h"

class ProjectCommandCreateSequence : public ProjectCommand
{
public:
    
    /** Create from autofolder */
    ProjectCommandCreateSequence(model::FolderPtr folder);

    /** Create new sequence in folder */
    ProjectCommandCreateSequence(model::FolderPtr folder, wxString name);

    ~ProjectCommandCreateSequence();
    
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

#endif // PROJECT_COMMAND_CREATE_SEQUENCE_H
