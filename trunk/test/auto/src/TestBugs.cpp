#include "TestBugs.h"

#include <wx/uiaction.h>
#include <boost/foreach.hpp>
#include "FixtureProject.h"
#include "HelperTimeline.h"

namespace test {

void TestBugs::setUp()
{
}

void TestBugs::tearDown()
{
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestBugs::testVideoDecodingError()
{
    StartTestSuite();
    FixtureProject mProjectFixture;
    mProjectFixture.init();
    TrimRight(VideoClip(0,3),4);
    TrimRight(VideoClip(0,3),10);
    TrimRight(VideoClip(0,3),15);
    TrimRight(VideoClip(0,3),20);
    mProjectFixture.destroy();

}

void TestBugs::testHangupAfterResettingDetailsView()
{
    FixtureProject mProjectFixture;
    mProjectFixture.init();
    StartTestSuite();
    Click(VTopQuarterHCenter(VideoClip(0,2)));
    DeselectAllClips();
    mProjectFixture.destroy();
}

} // namespace