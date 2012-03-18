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
#include "Intervals.h"
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
//    Zoom Level(3);
//    PrepareSnapping(true);

    StartTestSuite();
    Zoom level(3);
    {
        StartTest("Reduce size of second and third clip to be able to create transitions");
        TrimRight(VideoClip(0,1),30,false);
        ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
        TrimLeft(VideoClip(0,3),30,false); // Note: the trim of clip 1 causes clip 2 to become clip 3 (clip 2 is empty space)
        ASSERT(!VideoClip(0,3)->isA<model::EmptyClip>());
        ASSERT(!VideoClip(0,4)->isA<model::EmptyClip>());
        ASSERT_MORE_THAN_ZERO(VideoClip(0,1)->getMaxAdjustEnd())(VideoClip(0,1));
        ASSERT_LESS_THAN_ZERO(VideoClip(0,3)->getMinAdjustBegin())(VideoClip(0,2));
        // Make transitions between clips 2 and 3
        Move(RightCenter(VideoClip(0,1)));
        Type('c');
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT(VideoClip(0,3)->isA<model::EmptyClip>());
        waitForIdle();
        Move(LeftCenter(VideoClip(0,4)));
        Type('c');
        ASSERT(VideoClip(0,4)->isA<model::Transition>());
        ASSERT(!VideoClip(0,5)->isA<model::EmptyClip>());
        DragAlignLeft(Center(VideoClip(0,5)),RightPixel(VideoClip(0,2)));
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        ASSERT(VideoClip(0,3)->isA<model::Transition>());
        Scrub(LeftPixel(VideoTransition(0,2)) - 5, RightPixel(VideoTransition(0,3)) + 5);
        Play(LeftPixel(VideoTransition(0,2)) - 2, 1500); // -2: Also take some frames from the left clip
    }
    {
        StartTest("Drag a clip just on top of the right transition.");
        // This effectively removes that right transition and (part) of the clip to its right.
        // The clips to the left of the removed transition must remain unaffected.
        pts cliplength = VideoClip(0,1)->getLength();
        pts transitionlength = VideoClip(0,2)->getLength();
        pts length = VideoClip(0,8)->getLength();
        DragAlignLeft(Center(VideoClip(0,8)),LeftPixel(VideoClip(0,3)));
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),cliplength);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),transitionlength);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),length);
        Undo();
    }

    //wxString sFile( "scene'20100102 12.32.48.avi" ); // Should be a file also in the autofolder
    // model::NodePtr file = mProjectFixture.mAutoFolder->find(sFile).front();
    // wxPoint root = findNode(mProjectFixture.mRoot);
    // wxPoint from = findNode(file);
    // MoveWithinWidget(from,getProjectView().GetScreenPosition());
    // DragFromProjectViewToTimeline(from, Center(VideoClip(0,2)));
    triggerMenu(ID_ADDVIDEOTRACK);
    triggerMenu(ID_ADDAUDIOTRACK);pause();
    {
        StartTest("Shift trim with an empty other track (prepare for boundaries tests).");
        ShiftTrim(LeftCenter (VideoClip(0,3)),Center(VideoClip(0,3))-wxPoint(20,0)); // Bug occured: Shift trimming with an other track that did not have clips caused an 0-ptr error
        ShiftTrim(RightCenter(VideoClip(0,3)),Center(VideoClip(0,3))+wxPoint(20,0));
        Drag(Center(VideoClip(0,2)),wxPoint(HCenter(VideoClip(0,2)),VCenter(VideoTrack(1))),false,true,false);
        ControlDown();
        Move(Center(AudioClip(0,2)));
        ControlUp();
        Drag(Center(AudioClip(0,2)),wxPoint(HCenter(AudioClip(0,2)),VCenter(AudioTrack(1))),false,false,true);
        pause();
        Undo();
    }
    {

    }
    pause();

    // todo snapto also for trimming

}

} // namespace