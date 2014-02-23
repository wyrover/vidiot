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

#ifndef HELPER_FILE_SYSTEM_H
#define HELPER_FILE_SYSTEM_H

namespace test {

class RandomTempDir;
typedef boost::shared_ptr<RandomTempDir> RandomTempDirPtr;

class RandomTempDir : boost::noncopyable
{
public:

    explicit RandomTempDir(bool cleanup = true);
    RandomTempDir(wxFileName parentDir, bool cleanup); ///< Make subdir
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

/// Get the path to the topmost test folder
/// \return path to vidiot/trunk dir
wxFileName getTestPath();

/// Get the path to the folder containing the test input files
/// \return path to input files
wxFileName getTestFilesPath();

/// Get the input files in the test input folder
/// \return list of paths to input files
model::IPaths getListOfInputFiles();

/// \return path to a still image
wxFileName getStillImagePath();

/// Get the contents of the given path
/// \param path path to file on disk
/// \return contents of given file
wxString getFileContents(wxFileName path);

} // namespace

#endif