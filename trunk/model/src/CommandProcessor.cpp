// Copyright 2013-2015 Eric Raijmakers.
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

#include "CommandProcessor.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// wxCommandProcessor
//////////////////////////////////////////////////////////////////////////

bool CommandProcessor::Redo()
{
    ASSERT_MORE_THAN_ZERO(mRedo);
    mRedo--;
    return wxCommandProcessor::Redo();
}

bool CommandProcessor::Submit(wxCommand *command, bool storeIt)
{
    mRedo = 0;
    return wxCommandProcessor::Submit(command,storeIt);
}

bool CommandProcessor::Undo()
{
    mRedo++;
    return wxCommandProcessor::Undo();
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

std::vector<wxCommand*> CommandProcessor::getUndoHistory() const
{
    std::vector<wxCommand*> result;
    for (wxCommand* c : GetCommands().AsVector<wxCommand*>())
    {
        result.push_back(c);
    }

    // Remove any 'Undone' commands.
    ASSERT_LESS_THAN_EQUALS(mRedo, result.size())(result);
    size_t undone{ mRedo };
    while (undone-- > 0)
    {
        result.pop_back();
    }
    return result;
}


} // namespace