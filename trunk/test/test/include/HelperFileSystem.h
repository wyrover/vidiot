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

#pragma once

namespace test {

class RandomTempDir : boost::noncopyable
{
public:

    explicit RandomTempDir(bool cleanup = true);
    explicit RandomTempDir(wxFileName path); ///< Used to reuse this object, but with a fixed path. \note path is not removed.
    explicit RandomTempDir(wxFileName parentDir, bool cleanup); ///< Make subdir
    virtual ~RandomTempDir();

    static RandomTempDirPtr generate(bool cleanup = true);
    RandomTempDirPtr generateSubDir();

    wxFileName getFileName() const;

private:

    void makeDir();

    wxString mFullPath; ///< Useful when debugging (with wxFileName  it's not easy to extract the full path)
    wxFileName mFileName;
    bool mCleanup;

    static int sDirCount;
};

/// Get the path to the sources
/// \return path to the root of the source tree
wxFileName getSourceRoot();

/// Get the path to the topmost test folder
/// \return path to vidiot/trunk dir
wxFileName getTestPath();

/// Get the path to the folder containing test input files
/// \param subdir the path to this subdir (under test) is returned
/// \return path to input files
wxFileName getTestFilesPath(wxString subdir = "input");

/// Get the input files in the test input folder
/// \return list of paths to input files
model::IPaths getListOfInputPaths(wxFileName path = getTestFilesPath());

/// Get the input files in the test input folder
/// \return list of paths to input files as strings
wxStrings getListOfInputPathsAsStrings(wxFileName path = getTestFilesPath());

/// Get the input files in the test input folder
/// \return list of paths to input files as file names
wxFileNames getListOfInputPathsAsFileNames(wxFileName path = getTestFilesPath());

/// \return path to a still image
wxFileName getStillImagePath();

/// Get the contents of the given path
/// \param path path to file on disk
/// \return contents of given file
wxString getFileContents(wxFileName path);

/// Get the contents of the save file at the 
/// given path and remove any parameters that 
/// should not cause a comparison to fail.
/// \param path path to file on disk
/// \return contents of given file
wxString getSavedFileContents(wxFileName path);

void OpenFileExplorer(wxFileName dir);

} // namespace
