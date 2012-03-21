#include "TestOnce.h"

#include <wx/uiaction.h>
#include "typeinfo.h"

#include <wx/gdicmn.h>
#include <boost/foreach.hpp>
#include "AudioClip.h"
#include "AudioTrack.h"
#include "AutoFolder.h"
#include "ClipView.h"
#include "Config.h"
#include "CreateTransition.h"
#include "DeleteSelectedClips.h"
#include "Dialog.h"
#include "Dump.h"
#include "EmptyClip.h"
#include "ExecuteDrop.h"
#include "FixtureApplication.h"
#include "HelperApplication.h"
#include "HelperProjectView.h"
#include "HelperTimeline.h"
#include "HelperTimelineAssert.h"
#include "HelperTimelinesView.h"
#include "HelperTransition.h"
#include "HelperWindow.h"
#include "IClip.h"
#include "Intervals.h"
#include "Layout.h"
#include "MousePointer.h"
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
#include "VideoClip.h"
#include "VideoTrack.h"
#include "VideoTransition.h"
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
    getTimeline().getDump().dump();
    //    PrepareSnapping(true);
    StartTestSuite();
    Zoom level(2);
    {
        StartTest("Trim: EndTrim: Enlarge the last clip in a track (there is no empty clip after it anymore)");
         TrimRight(VideoClip(0,3),40,false);
         pts length = VideoClip(0,3)->getLength();
         DragAlignLeft(Center(VideoClip(0,3)),RightPixel(VideoClip(0,7)));
         TrimRight(VideoClip(0,7),-20,false);
         ASSERT_MORE_THAN(VideoClip(0,7)->getLength(), length);
         Undo();
         Undo();
         Undo();

    }

       //wxString sFile( "scene'20100102 12.32.48.avi" ); // Should be a file also in the autofolder
    // model::NodePtr file = mProjectFixture.mAutoFolder->find(sFile).front();
    // wxPoint root = findNode(mProjectFixture.mRoot);
    // wxPoint from = findNode(file);
    // MoveWithinWidget(from,getProjectView().GetScreenPosition());
    // DragFromProjectViewToTimeline(from, Center(VideoClip(0,2)));
    triggerMenu(ID_ADDVIDEOTRACK);
    triggerMenu(ID_ADDAUDIOTRACK);
    {
        // todo move these cases to other test classes
        // todo below replace all the shifttrim statements with
        // TrimRIght and TrimLeft statements......
        StartTest("ShiftTrim: BeginTrim: Shorten: with an empty other track.");
        pts previouslength = VideoClip(0,4)->getLength();
        ShiftTrim(LeftCenter (VideoClip(0,4)),Center(VideoClip(0,4))-wxPoint(20,0)); // Bug occured: Shift trimming with an other track that did not have clips caused 0-ptr error
        ASSERT_LESS_THAN_ZERO(VideoClip(0,4)->getMinAdjustBegin());
        ASSERT_MORE_THAN_ZERO(VideoClip(0,4)->getMaxAdjustBegin());
        ASSERT_LESS_THAN(VideoClip(0,4)->getLength(),previouslength);
        previouslength = VideoClip(0,4)->getLength();
        StartTest("ShiftTrim: BeginTrim: Enlarge: with an empty other track.");
        previouslength = VideoClip(0,4)->getLength();
        ShiftTrim(LeftCenter (VideoClip(0,4)),LeftCenter(VideoClip(0,4))-wxPoint(10,0)); // Bug occured: Shift trimming with an other track that did not have clips caused 0-ptr error
        ASSERT_LESS_THAN_ZERO(VideoClip(0,4)->getMinAdjustBegin());
        ASSERT_MORE_THAN_ZERO(VideoClip(0,4)->getMaxAdjustBegin());
        ASSERT_MORE_THAN(VideoClip(0,4)->getLength(),previouslength);
        StartTest("ShiftTrim: EndTrim: Shorten: with an empty other track.");
        previouslength = VideoClip(0,4)->getLength();
        ShiftTrim(RightCenter(VideoClip(0,4)),Center(VideoClip(0,4))+wxPoint(20,0)); // Bug occured: Shift trimming with an other track that did not have clips caused 0-ptr error
        ASSERT_LESS_THAN_ZERO(VideoClip(0,4)->getMinAdjustEnd());
        ASSERT_MORE_THAN_ZERO(VideoClip(0,4)->getMaxAdjustEnd());
        ASSERT_LESS_THAN(VideoClip(0,4)->getLength(),previouslength);
        StartTest("ShiftTrim: EndTrim: Enlarge: with an empty other track.");
        previouslength = VideoClip(0,4)->getLength();
        ShiftTrim(RightCenter(VideoClip(0,4)),RightCenter(VideoClip(0,4))+wxPoint(10,0)); // Bug occured: Shift trimming with an other track that did not have clips caused 0-ptr error
        ASSERT_LESS_THAN_ZERO(VideoClip(0,4)->getMinAdjustEnd());
        ASSERT_MORE_THAN_ZERO(VideoClip(0,4)->getMaxAdjustEnd());
        ASSERT_MORE_THAN(VideoClip(0,4)->getLength(),previouslength);
        StartTest("Make a clip in another track (preparation).");
        previouslength = VideoClip(0,4)->getLength();
        DragToTrack(1,VideoClip(0,3),AudioClip(0,3));
        ASSERT_VIDEOTRACK1(EmptyClip)                      (VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip)(AudioClip);
        ASSERT_AUDIOTRACK1(EmptyClip)                      (AudioClip);
        ASSERT_LESS_THAN_ZERO(VideoClip(0,4)->getMinAdjustBegin());
        ASSERT_MORE_THAN_ZERO(VideoClip(0,4)->getMaxAdjustBegin());
        ASSERT_LESS_THAN_ZERO(VideoClip(0,4)->getMinAdjustEnd());
        ASSERT_MORE_THAN_ZERO(VideoClip(0,4)->getMaxAdjustEnd());
        StartTest("Move the clip in the other track slightly over the begin of the tested clip (preparation).");
        Drag(Center(VideoClip(0,4)),RightCenter(VideoClip(0,3)));
        StartTest("ShiftTrim: EndTrim: Shorten: with another track that is shorter than the trim position (this imposes a lower bound on the shift).");
        pts diff = VideoClip(0,4)->getRightPts() - VideoTrack(1)->getLength();
        pts track0len = VideoTrack(0)->getLength();
        ShiftTrim(RightCenter(VideoClip(0,4)), LeftCenter(VideoClip(0,4)));
        ASSERT_EQUALS(VideoClip(0,4)->getRightPts(),VideoTrack(1)->getLength());
        ASSERT_EQUALS(VideoTrack(0)->getLength(),track0len - diff);
        Undo(); // Undo the trim
        StartTest("ShiftTrim: BeginTrim: Shorten: with another track that has a clip on the trim position (no trim possible).");
        previouslength = VideoClip(0,4)->getLength();
        ShiftTrim(LeftCenter(VideoClip(0,4)), Center(VideoClip(0,4)));
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), previouslength);
        StartTest("ShiftTrim: BeginTrim: Enlarge: with another track that has a clip on the trim position (no trim possible).");
        ShiftTrim(LeftCenter(VideoClip(0,4)), LeftCenter(VideoClip(0,1)));
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), previouslength);
        StartTest("Move the clip in the other track over the end of the tested clip (preparation).");
        Drag(Center(VideoClip(1,1)),wxPoint(RightPixel(VideoClip(0,4)),VCenter(VideoTrack(1))));
        StartTest("ShiftTrim: EndTrim: Shorten: with another track that has a clip on the trim position (no trim possible).");
        ShiftTrim(RightCenter(VideoClip(0,4)), Center(VideoClip(0,4)));
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), previouslength);
        StartTest("ShiftTrim: EndTrim: Enlarge: with another track that has a clip on the trim position (no trim possible).");
        ShiftTrim(RightCenter(VideoClip(0,4)), RightCenter(VideoClip(0,5)));
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), previouslength);
        StartTest("Ensure that there's a clip in another track before AND after AND 'inbetween' the clip under test (preparation).");
        Undo();
        Undo();
        DragToTrack(1,VideoClip(0,5),AudioClip(0,5));
        TrimRight(VideoClip(0,6),40);
        StartTest("ShiftTrim: BeginTrim: Shorten: other track space imposes a trim restriction.");
        DragToTrack(1,VideoClip(0,6),AudioClip(0,6));
        PrepareSnapping(false);
        Drag(Center(VideoClip(1,1)),Center(VideoClip(1,1))-wxPoint(8,0));
        Drag(Center(VideoClip(1,3)),Center(VideoClip(1,3))+wxPoint(8,0));
        DragAlignLeft(Center(VideoClip(1,4)),LeftPixel(VideoClip(0,4))+20);
        ASSERT_VIDEOTRACK1(EmptyClip)                   (VideoClip)(EmptyClip)(VideoClip)(EmptyClip)(VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(       VideoClip      )(EmptyClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(       AudioClip      )(EmptyClip);
        ASSERT_AUDIOTRACK1(EmptyClip)                   (AudioClip)(EmptyClip)(AudioClip)(EmptyClip)(AudioClip);
        previouslength = VideoClip(0,4)->getLength();
        pts minbegin = VideoClip(0,4)->getMinAdjustBegin();
        pts maxbegin = VideoClip(0,4)->getMaxAdjustBegin();
        pts minend = VideoClip(0,4)->getMinAdjustEnd();
        pts maxend = VideoClip(0,4)->getMaxAdjustEnd();
        StartTest("ShiftTrim: BeginTrim: Shorten: other track space imposes a trim restriction.");
        pts maxadjust = VideoClip(1,3)->getLeftPts() - VideoClip(0,4)->getLeftPts();
        ShiftTrim(LeftCenter(VideoClip(0,4)), RightCenter(VideoClip(0,4)));
        ASSERT_MORE_THAN(VideoClip(0,4)->getLength(), previouslength - maxbegin); // Can't trim to the max due to the restriction in the other track
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), previouslength - maxadjust);
        ASSERT_EQUALS(VideoClip(0,4)->getLeftPts(),VideoClip(1,3)->getLeftPts());
        Undo();
        StartTest("ShiftTrim: BeginTrim: Enlarge: other track space imposes NO trim restriction.");
        ShiftTrim(LeftCenter(VideoClip(0,4)), LeftCenter(VideoClip(0,1)));
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), previouslength - minbegin); // Note: minbegin < 0
        Undo();
        StartTest("ShiftTrim: EndTrim: Shorten: other track space imposes a trim restriction.");
        pts minadjust = VideoClip(1,3)->getRightPts() - VideoClip(0,4)->getRightPts();
        ShiftTrim(RightCenter(VideoClip(0,4)), LeftCenter(VideoClip(0,4)));
        ASSERT_MORE_THAN(VideoClip(0,4)->getLength(), previouslength + minend); // Can't trim to the max due to the restriction in the other track
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), previouslength + minadjust ); // Note: minadjust < 0
        ASSERT_EQUALS(VideoClip(0,4)->getRightPts(),VideoClip(1,3)->getRightPts());
        Undo();
        StartTest("ShiftTrim: EndTrim: Enlarge: other track space imposes NO trim restriction.");
        ShiftTrim(RightCenter(VideoClip(0,4)), RightCenter(VideoClip(0,5)));
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), previouslength + maxend);
        Undo();
    }
    // todo snapto also for trimming

}

} // namespace