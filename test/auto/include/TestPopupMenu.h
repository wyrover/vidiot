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