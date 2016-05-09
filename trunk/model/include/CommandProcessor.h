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

    bool CanUndo() const override;
    bool CanRedo() const override;

    bool Redo() override;
    bool Submit (wxCommand *command, bool storeIt=true) override;
    bool Undo() override;

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    std::vector<wxCommand*> getUndoHistory() const;
    void registerPlayer(IPlayer* player);
    void unregisterPlayer(IPlayer* player);

    /// Used for ensuring no undo/redo is triggered when 'busy' with another edit.
    /// Typically used to avoid changing the model after starting a drag and drop
    /// or trim operation and pressing CTRL-Z while the drag/trim is active.
    /// The Undo causes the underlying model to be changed, resulting in a 
    /// (possible) crash when finalizing the drop/trim.
    /// \param enable iff true, Undo/Redo is allowed.
    void enableUndoRedo(bool enable);

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mUndoRedoEnabled = true; ///< True iff the document may be changed via undo/redo.
    size_t mRedo = 0; ///< Number of commands in the command list that is redo-able.
    std::vector<IPlayer*> mPlayers;
};

} // namespace
