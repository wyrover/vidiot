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

#include "RootCommand.h"

#include "ProjectModification.h"

namespace cmd {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

RootCommand::RootCommand()
    :   wxCommand(true)
    ,   mCommandName(_("Unnamed command"))
{
}

RootCommand::~RootCommand()
{
}

//////////////////////////////////////////////////////////////////////////
// SUBMIT
//////////////////////////////////////////////////////////////////////////

void RootCommand::submit()
{
    // todo give sequences their own undo history AND command processors by using child document functionality of wxDocument.
    // then pause the associated players automatically, whenever a command is done/undone/etc.
    // Note: this should not only be done for submitted commands but also for partial edits like which is done for trimclip::update()!!!

    model::ProjectModification::submit(this);
}

bool RootCommand::isPossible() const
{
    return true;
}


//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

wxString RootCommand::GetName() const
{
    return mCommandName;
}

} // namespace