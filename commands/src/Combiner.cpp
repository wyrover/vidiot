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

#include "Combiner.h"

namespace cmd {

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
        delete mCommands.back();
        mCommands.pop_back();
    }
}

//////////////////////////////////////////////////////////////////////////
// WXWIDGETS DO/UNDO INTERFACE
//////////////////////////////////////////////////////////////////////////

bool Combiner::Do()
{
    for ( wxCommand* command : mCommands )
    {
        command->Do();
    }
    return true;
}

bool Combiner::Undo()
{
    for( wxCommand* command : boost::adaptors::reverse( mCommands ) )
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
    mCommands.emplace_back(command);
    mCommandName = command->GetName();
}

void Combiner::setName(const wxString& name)
{
    mCommandName = name;
}

} // namespace