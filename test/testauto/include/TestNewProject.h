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

#include "TestAuto.h"

namespace test {

class TestNewProject : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestNewProject>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // TEST CASES
    //////////////////////////////////////////////////////////////////////////

    /// Test generation of a new project by starting with an empty folder.
    /// Test selecting the folder option on the start page.
    void testEmptyFolder();

    /// Test generation of a new project by starting with an folder containing usable media files.
    /// Test reading the folder option from the config as default selection on the start page.
    /// Test reading the default video frame rate from the provided files.
    /// Test reading the default audio sample rate from the provided files.
    void testFolder();

    /// Test generation of a new project by starting with a set of files of which no file is usable.
    /// Test selecting the files option on the start page.
    void testNoUsableFiles();

    /// Test generation of a new project by starting with a set of files.
    /// Test reading the files option from the config as default selection on the start page.
    void testFiles();

    /// Test generation of a blank new project.
    /// Test selecting the blank option on the start page.
    void testBlank();

    /// Test aborting generation of a blank new project.
    /// Test reading the blank option from the config as default selection on the start page.
    void testCancel();
};

} // namespace

using namespace test;
