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

#ifndef ROOT_COMMAND_H
#define ROOT_COMMAND_H

namespace command {

class RootCommand : public wxCommand
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    RootCommand();
    virtual ~RootCommand();

    //////////////////////////////////////////////////////////////////////////
    // SUBMIT
    //////////////////////////////////////////////////////////////////////////

    void submit();

    /// \return true if the command may be submit.
    /// To be overridden in derived classes such that they can indicate whether
    /// submitting is possible or not.
    /// Default implementation always returns true.
    virtual bool isPossible();

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    wxString GetName() const override;

protected:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    wxString mCommandName;
};

} // namespace

#endif // ROOT_COMMAND_H