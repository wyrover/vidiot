// Copyright 2013-2016 Eric Raijmakers.
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

#pragma once

#include "UtilSingleInstance.h"

struct IPlayer;

namespace model {

class CommandProcessor
    :   public wxCommandProcessor
    ,   public SingleInstance<CommandProcessor>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    CommandProcessor() = default;
    virtual ~CommandProcessor() = default;

    //////////////////////////////////////////////////////////////////////////
    // wxCommandProcessor
    //////////////////////////////////////////////////////////////////////////

    bool Redo() override;
    bool Submit (wxCommand *command, bool storeIt=true) override;
    bool Undo() override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    std::vector<wxCommand*> getUndoHistory() const;
    void registerPlayer(IPlayer* player);
    void unregisterPlayer(IPlayer* player);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    size_t mRedo = 0; ///< Number of commands in the command list that is redo-able.
    std::vector<IPlayer*> mPlayers;
};

} // namespace
