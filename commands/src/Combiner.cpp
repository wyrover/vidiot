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

#include "Combiner.h"

namespace command {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

Combiner::Combiner()
    :   RootCommand()
{
}

Combiner::~Combiner()
{
    while (!mCommands.empty())
    {
        delete mCommands.front();
        mCommands.pop_front();
    }
}

//////////////////////////////////////////////////////////////////////////
// WXWIDGETS DO/UNDO INTERFACE
//////////////////////////////////////////////////////////////////////////

bool Combiner::Do()
{
    BOOST_FOREACH( wxCommand* command, mCommands )
    {
        command->Do();
    }
    return true;
}

bool Combiner::Undo()
{
    BOOST_REVERSE_FOREACH( wxCommand* command, mCommands )
    {
        command->Undo();
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

void Combiner::add(wxCommand* command)
{
    mCommands.push_back(command);
    mCommandName = command->GetName();
}

void Combiner::setName(wxString name)
{
    mCommandName = name;
}

} // namespace