#ifndef TEST_AUTO_FOLDER_H
#define TEST_AUTO_FOLDER_H

namespace test
{

class TestAutoFolder : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestAutoFolder>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // TEST CASES
    //////////////////////////////////////////////////////////////////////////

    /// Test adding an autofolder that already contains files
    void testAddAutoFolder();

};

}
using namespace test;

#endif // TEST_AUTO_FOLDER_H