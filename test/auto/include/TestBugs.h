#ifndef TEST_BUGS_H
#define TEST_BUGS_H

#include "SuiteCreator.h"

namespace test
{

/// This class tests scenarios that were problematic at one point.
class TestBugs : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestBugs>
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

    /// Video decoding was not properly initialized in all cases. Particularly, when moving
    /// to one of the last frames (thus, near EOF) and then starting the decode, it was possible
    /// that the reading of all packets had already been finished (eof set for the video file)
    /// and a subsequent call to startDecoding* was returned immediately because of the EOF flag.
    /// Bug was fixed by only not starting a renewed 'reading of packets'. Decoding would be started,
    /// regardless the EOF flag.
    void testVideoDecodingError();

    /// A hangup occurred after resetting the details view multiple times. Apparently, calling
    /// DestroyChildren too often seemed to (note: seemed to) cause this.
    void testHangupAfterResettingDetailsView();

    /// The details view was sometimes not shown after moving the cursor, and then clicking
    /// on a clip.
    void testDetailsNotShownAfterMovingTimelineCursor();

};
}

using namespace test;

#endif // TEST_BUGS_H