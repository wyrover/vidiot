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

#pragma once

#include "Test.h"

namespace test {

class TestWatch: public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestWatch>
{
public:

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

    void setUp();
    void tearDown();

    //////////////////////////////////////////////////////////////////////////
    // TEST CASES
    //////////////////////////////////////////////////////////////////////////

    void testRemovedWatchedFolder();
    void testAddAndRemoveFileToWatchedAutoFolder();
    void testAddAndRemoveFileToWatchedNonAutoFolder();
    void testRemoveProjectViewFolderContainingFileOnDisk();

    /// See [#233]: File watching was not properly initialized when opening
    /// a project from disk.
    void testWatchAutoFolderAfterOpenProject();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    model::FolderPtr mRoot;
    RandomTempDirPtr mTempDir;
    wxFileName mTempDirName;
    model::IPaths mInputFiles;

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    model::FolderPtr setup();
};

} // namespace

using namespace test;
