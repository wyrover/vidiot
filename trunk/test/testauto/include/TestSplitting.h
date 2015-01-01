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

#ifndef TEST_SPLITTING_H
#define TEST_SPLITTING_H

#include "TestAuto.h"

namespace test
{
class TestSplitting : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestSplitting>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    virtual void setUp();       ///< Called before each test.
    virtual void tearDown();    ///< Called after each test.

    //////////////////////////////////////////////////////////////////////////
    // TEST CASES
    //////////////////////////////////////////////////////////////////////////

    /// Test the splitting of the sequence at the position of the cursor
    void testSplitting();

    /// Test the splitting of the sequence at the position of the cursor
    /// while playback is active.
    void testSplittingDuringPlayback();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    FixtureProject mProjectFixture;
};
}
using namespace test;

#endif