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

#ifndef PROJECT_MODIFICATION_H
#define PROJECT_MODIFICATION_H

namespace command {
    class RootCommand;
}

namespace model {

/// Trigger a modification of the project. Made into a separate interface
/// file to avoid lots of files being dependent on Project
struct ProjectModification
{
    /// Set the modified status of the project.
    /// Enables Save button and causes a 'save?' dialog
    /// before closing the project.
    static void trigger();

    /// Submit a change to the project.
    /// That includes triggering the modified status of the project.
    /// \see trigger()
    /// \param comand
    static void submit(command::RootCommand* c);

    /// Submit a change to the project, but only if the change isPossible.
    /// That includes triggering the modified status of the project.
    /// \note if the command is not possible, it is deleted
    /// \see trigger()
    /// \param comand
    /// \return true if the command was submit
    static bool submitIfPossible(command::RootCommand* c);
};

} // namespace

#endif // I_PROJECT_MODIFICATION_H