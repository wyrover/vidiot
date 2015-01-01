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

#include "UtilLog.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

CommandProcessor::CommandProcessor()
    :   wxCommandProcessor()
    ,   mUndoSize(0)
{
}

CommandProcessor::~CommandProcessor()
{
}

//////////////////////////////////////////////////////////////////////////
// wxCommandProcessor
//////////////////////////////////////////////////////////////////////////

bool CommandProcessor::Redo()
{
    mUndoSize++;
    ASSERT_MORE_THAN_ZERO(mUndoSize);
    return wxCommandProcessor::Redo();
}

bool CommandProcessor::Submit (wxCommand *command, bool storeIt)
{
    mUndoSize++;
    ASSERT_MORE_THAN_ZERO(mUndoSize);
    return wxCommandProcessor::Submit(command,storeIt);
}

bool CommandProcessor::Undo()
{
    mUndoSize++;
    ASSERT_MORE_THAN_EQUALS_ZERO(mUndoSize);
    return wxCommandProcessor::Undo();
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

int CommandProcessor::getUndoSize() const
{
    ASSERT_MORE_THAN_EQUALS_ZERO(mUndoSize);
    return mUndoSize;
}

} // namespace