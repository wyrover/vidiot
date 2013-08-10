// Copyright 2013 Eric Raijmakers.
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

#ifndef TEST_POPUPMENU_H
#define TEST_POPUPMENU_H

namespace test
{
class TestPopupMenu : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestPopupMenu>
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

    /// Test adding transitions via the popup menu.
    void testAddTransitions();

    /// Test removing one empty area via the popup menu.
    void testRemoveOneEmptyInterval();

    /// Test the special handling for multiple right mouse clicks, and
    /// for starting the scrolling immediately from when a popup menu is shown.
    /// Also tests that the various timeline view parts are extended when
    /// the total sequence length is increase.
    void testAdvancedRightClickScenarios();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    FixtureProject mProjectFixture;
};
}
using namespace test;

#endif // TEST_POPUPMENU_H