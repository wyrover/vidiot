// Copyright 2013 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

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