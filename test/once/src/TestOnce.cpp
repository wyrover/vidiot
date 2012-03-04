#include "TestOnce.h"

#include <wx/uiaction.h>
#include "typeinfo.h"

//#include <boost/algorithm/string.hpp>
#include <wx/gdicmn.h>
#include <boost/foreach.hpp>
#include "AudioTrack.h"
#include "AutoFolder.h"
#include "ClipView.h"
#include "Config.h"
#include "CreateTransition.h"
#include "DeleteSelectedClips.h"
#include "Dialog.h"
#include "EmptyClip.h"
#include "AudioClip.h"
#include "ExecuteDrop.h"
#include "FixtureApplication.h"
#include "HelperApplication.h"
#include "HelperProjectView.h"
#include "HelperTimeline.h"
#include "HelperTimelineAssert.h"
#include "HelperTimelinesView.h"
#include "HelperTransition.h"
#include "HelperWindow.h"
#include "Layout.h"
#include "MousePointer.h"
#include "IClip.h"
#include "PositionInfo.h"
#include "ProjectViewCreateAutoFolder.h"
#include "ProjectViewCreateSequence.h"
#include "Selection.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "Timeline.h"
#include "Track.h"
#include "Transition.h"
#include "Trim.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "VideoTrack.h"
#include "VideoTransition.h"
#include "VideoClip.h"
#include "ViewMap.h"
#include "Window.h"
#include "Zoom.h"
#include "ids.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestOnce::setUp()
{
    mProjectFixture.init();
}

void TestOnce::tearDown()
{
    mProjectFixture.destroy();
}

auto PrepareSnapping = [](bool enableSnapping)
{
    checkMenu(ID_SNAP_CLIPS, enableSnapping);
    checkMenu(ID_SNAP_CURSOR, enableSnapping);
    DeselectAllClips();
};

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestOnce::testOnce()
{
        //BREAK();
    StartTestSuite();
    Zoom Level(1);

    Move(OnTimescaleAbove(Center(VideoClip(0,2))));

    StartTest("Make an interval from left to right and click 'delete all marked intervals'");
    ShiftDrag(OnTimescaleAbove(Center(VideoClip(0,1))),OnTimescaleAbove(Center(VideoClip(0,2))));pause();
    triggerMenu(ID_DELETEMARKED);
    ASSERT_EQUALS(VideoClip(0,0)->getLength(),      mProjectFixture.OriginalLengthOfVideoClip(0,0));
    ASSERT_LESS_THAN(VideoClip(0,1)->getLength(),   mProjectFixture.OriginalLengthOfVideoClip(0,1));
    ASSERT_LESS_THAN(VideoClip(0,2)->getLength(),   mProjectFixture.OriginalLengthOfVideoClip(0,2));
    ASSERT_EQUALS(VideoClip(0,3)->getLength(),      mProjectFixture.OriginalLengthOfVideoClip(0,3));
    Undo();
    Undo();
    StartTest("Make an interval from right to left and click 'delete all marked intervals'");
    ShiftDrag(OnTimescaleAbove(Center(VideoClip(0,2))),OnTimescaleAbove(Center(VideoClip(0,1))));
    triggerMenu(ID_DELETEMARKED);
    ASSERT_EQUALS(VideoClip(0,0)->getLength(),      mProjectFixture.OriginalLengthOfVideoClip(0,0));
    ASSERT_LESS_THAN(VideoClip(0,1)->getLength(),   mProjectFixture.OriginalLengthOfVideoClip(0,1));
    ASSERT_LESS_THAN(VideoClip(0,2)->getLength(),   mProjectFixture.OriginalLengthOfVideoClip(0,2));
    ASSERT_EQUALS(VideoClip(0,3)->getLength(),      mProjectFixture.OriginalLengthOfVideoClip(0,3));
    Undo();
    Undo();
    pause();

    // todo test move audiovideo divider... test move track divider
}

} // namespace