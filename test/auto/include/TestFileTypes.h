#ifndef TEST_FILE_TYPES_H
#define TEST_FILE_TYPES_H

namespace model {
class IPath;
typedef boost::shared_ptr<IPath> IPathPtr;
typedef std::list<IPathPtr> IPaths;
class Folder;
typedef boost::shared_ptr<Folder> FolderPtr;
class Sequence;
typedef boost::shared_ptr<Sequence> SequencePtr;
}

namespace test {

class TestFileTypes: public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestFileTypes>
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

    void testFileTypes_1_44100();
    void testFileTypes_2_44100();
    void testFileTypes_1_48000();
    void testFileTypes_2_48000();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    void executeTest();
};

} // namespace

using namespace test;

#endif // TEST_FILE_TYPES_H