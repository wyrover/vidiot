#ifndef COMBINE_COMMAND_H
#define COMBINE_COMMAND_H

#include "RootCommand.h"

namespace command {

/// Combine multiple commands into one entry in the Undo history
/// Simply add the commands. Only the last command is 'visible' in the undo history.
/// One undo action will 'undo' all the commands.
/// Name used in the Undo menu is the name of the last added command or the explicitly set name.
class Combiner : public RootCommand
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    Combiner();
    virtual ~Combiner();

    //////////////////////////////////////////////////////////////////////////
    // WXWIDGETS DO/UNDO INTERFACE
    //////////////////////////////////////////////////////////////////////////

    bool Do() override;
    bool Undo() override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    /// Add the given command to the list, and use it's name as the overall command name
    /// \param new command
    void add(wxCommand* command);

    /// Set the command name.
    /// \note any subsequent call to 'add' will replace this name with the name of that command.
    void setName(wxString name);

protected:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxString mCommandName;
    std::list< wxCommand* > mCommands;
};

} // namespace

#endif // COMBINE_COMMAND_H