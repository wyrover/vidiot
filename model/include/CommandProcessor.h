// Copyright 2013,2014 Eric Raijmakers.
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

#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include "UtilSingleInstance.h"

namespace model {

class CommandProcessor
    :   public wxCommandProcessor
    ,   public SingleInstance<CommandProcessor>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    CommandProcessor();
    virtual ~CommandProcessor();

    //////////////////////////////////////////////////////////////////////////
    // wxCommandProcessor
    //////////////////////////////////////////////////////////////////////////

    bool Redo() override;
    bool Submit (wxCommand *command, bool storeIt=true) override;
    bool Undo() override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    int getUndoSize() const; ///< \return maximum number of Undos that is currently possible

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    int mUndoSize; ///< Maximum number of Undos that is currently possible
};

} // namespace

#endif
