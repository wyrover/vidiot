// Copyright 2014-2016 Eric Raijmakers.
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

namespace test {

/// Create sequence with one file each time and then execute the given action.
/// \param pathToFiles path to test files
/// \param action action to execute after opening the sequence with the file
/// \param wait if true then waits until thumbnails/peaks have been generated before calling the action
void ExecuteOnAllFiles(wxString pathToFiles, std::function<void()> action, bool wait = false);

/// Create sequence with all files and then call the given action for each clip.
/// \param pathToFiles path to test files
/// \param action action to execute after opening the sequence with the file (parameter is clip number)
void AddClipsAndExecute(wxString pathToFiles, std::function<void(int)> action);

} // namespace
