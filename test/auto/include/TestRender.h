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

#ifndef TEST_ONCE_H
#define TEST_ONCE_H

namespace test
{

class TestRender : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestRender>
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

    void testChangeRenderSettings();
    void testRenderingPlayback();
    void testRenderingSplit();
    void testRenderingCodecs();

    /// This was a bug once. Caused by the clone not having the proper clip
    /// administration (getPrev/getNext failed when the transition started
    /// generating frames/chunks).
    void testRenderingTransition();

    void testRenderingTransformedClip();

private:

    FixtureProject mProjectFixture;
};

}
using namespace test;

#endif // TEST_ONCE_H