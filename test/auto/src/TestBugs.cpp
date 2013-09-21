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

#include "TestBugs.h"

#include "AudioChunk.h"
#include "AudioClip.h"
#include "AudioCompositionParameters.h"
#include "AudioTrack.h"
#include "DetailsClip.h"
#include "EmptyClip.h"
#include "HelperDetails.h"
#include "HelperPopupMenu.h"
#include "HelperTimeline.h"
#include "HelperTimelineAssert.h"
#include "HelperTimelineDrag.h"
#include "HelperTimelinesView.h"
#include "HelperTimelineTrim.h"
#include "HelperTransition.h"
#include "HelperWindow.h"
#include "IClip.h"
#include "ids.h"
#include "ProjectViewCreateSequence.h"
#include "Sequence.h"
#include "VideoClip.h"
#include "VideoCompositionParameters.h"
#include "VideoFrame.h"
#include "VideoTrack.h"

namespace test {

void TestBugs::setUp()
{
    mProjectFixture.init();
}

void TestBugs::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestBugs::testVideoDecodingError()
{
    StartTestSuite();
    TrimRight(VideoClip(0,3),4);
    TrimRight(VideoClip(0,3),10);
    TrimRight(VideoClip(0,3),15);
    TrimRight(VideoClip(0,3),20);
}

void TestBugs::testHangupAfterResettingDetailsView()
{
    StartTestSuite();
    Click(VTopQuarterHCenter(VideoClip(0,2)));
    DeselectAllClips();
}

void TestBugs::testDetailsNotShownAfterMovingTimelineCursor()
{
    StartTestSuite();
    Click(Center(VideoClip(0,3)));
    PositionCursor(HCenter(VideoClip(0,3)));
    Click(Center(VideoClip(0,3)));
    ASSERT_DETAILSCLIP(VideoClip(0,3));
}

void TestBugs::testLinkingErrorWhenDroppingOverBeginOfLinkedClip()
{
    StartTestSuite();
    triggerMenu(ID_ADDVIDEOTRACK);
    TrimLeft(VideoClip(0,4),40,false);
    Drag(From(Center(VideoClip(0,6))).To(wxPoint(RightPixel(VideoClip(0,4)),VCenter(getSequence()->getVideoTrack(1)))));
    ASSERT_EQUALS(VideoClip(0,5)->getLink(),AudioClip(0,6));
    TrimLeft(VideoClip(0,5),10,false); // This caused an assert, because there was a problem with this clip (video(0,5)) link.
}

void TestBugs::testErrorInGetNextHandlingForEmptyClips()
{
    StartTestSuite();
    triggerMenu(ID_ADDVIDEOTRACK);
    triggerMenu(ID_ADDVIDEOTRACK);

    DragToTrack(1,VideoClip(0,5),model::IClipPtr());
    Drag(From(Center(VideoClip(1,1))).To(wxPoint(HCenter(VideoClip(0,4)),VCenter(VideoClip(1,1)))));

    DragToTrack(2,VideoClip(0,6),model::IClipPtr());
    Drag(From(Center(VideoClip(2,1))).AlignLeft(LeftPixel(VideoClip(1,1))));

    Click(Center(VideoClip(1,1)));
    ClickTopLeft(DetailsClipView()->getScalingSlider()); // Give focus
    TypeN(6,WXK_PAGEUP);
    ClickTopLeft(DetailsClipView()->getPositionXSlider()); // Give focus
    TypeN(4,WXK_PAGEDOWN);
    ClickTopLeft(DetailsClipView()->getPositionYSlider()); // Give focus
    TypeN(4,WXK_PAGEDOWN);

    Click(Center(VideoClip(2,1)));
    ClickTopLeft(DetailsClipView()->getScalingSlider()); // Give focus
    TypeN(6,WXK_PAGEUP);
    ClickTopLeft(DetailsClipView()->getPositionXSlider()); // Give focus
    TypeN(4,WXK_PAGEUP);
    ClickTopLeft(DetailsClipView()->getPositionYSlider()); // Give focus
    TypeN(4,WXK_PAGEUP);

    PositionCursor(LeftPixel(VideoClip(2,1)) - 5);

    Play(LeftPixel(VideoClip(2,1)) - 3,2500); // Start before the clip, in the empty area. Due to a bug in 'getNext' handling for empty clips the clips after the empty area were not shown, or too late.

    Undo(9);
}

void TestBugs::testDraggingWithoutSelection()
{
    StartTestSuite();

    Click(Center(VideoClip(0,1))); // Select the clip
    ControlDown();
    LeftDown(); // Deselects the clip already
    Move(Center(VideoClip(0,3))); // Starts the drag without anything being selected: ASSERT at the time of the bug. Now the drag should be simply omitted.

    ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();

    ControlUp();
    LeftUp();
}

void TestBugs::testPlaybackLongTimeline()
{
    StartTestSuite();

     // todo implement after I found some way to easily make a very large sequence
     // Note: one or two large still images does not work
}

void TestBugs::testPlaybackEmptyClip()
{
    StartTestSuite();

    Click(Center(VideoClip(0,3)));
    Type(WXK_DELETE);
    PositionCursor(HCenter(VideoClip(0,3)));
    model::VideoFramePtr frame = boost::dynamic_pointer_cast<model::EmptyClip>(VideoClip(0,3))->getNextVideo(model::VideoCompositionParameters().setBoundingBox(wxSize(100,100)));
    ASSERT_NONZERO(frame);
    model::AudioChunkPtr chunk = boost::dynamic_pointer_cast<model::EmptyClip>(AudioClip(0,3))->getNextAudio(model::AudioCompositionParameters());
    ASSERT_NONZERO(chunk);
    // Note: do not pause() or press space at this point. The getNexts above 'mess up' the administration (audioclip is already at end, but the cursor position is not)
    Play(RightPixel(VideoClip(0,3)) - 3,2000);
}

void TestBugs::testPlaybackDoesNotStopAfterPressingShift()
{
    // todo after I found some way to easily make a very large sequence...
    // todo then also make a test case holding: lots of transitions, then do (for each transition) move to center frame, do one get next, then moveto next transition (to trigger *Transition::clean() omission bug...)
    StartTestSuite();
    Zoom level(2); // todo use zoom level(6)
    PositionCursor(HCenter(VideoClip(0,2)));
    Type(WXK_SPACE); // Start playback
    pause(1000);
    ShiftDown();
    pause(1000);
    ShiftUp();
    pause(1000);
    Type(WXK_SPACE); // Stop playback
    Click(Center(VideoClip(0,2)));
    Type(WXK_DELETE);
    ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(EmptyClip); // If playback doesn't stop the delete is ignored... thus, this actually checks that playback was stopped
}

void TestBugs::testTrimmingClipAdjacentToZeroLengthClipUsedForTransition()
{
    StartTestSuite();
    Zoom level(6);
    {
        StartTest("Clip to the right of the trim has length 0.");
        OpenPopupMenuAt(Center(VideoClip(0,1)));
        Type('o'); // fade &out
        ShiftTrim(LeftCenter(VideoClip(0,1)), RightCenter(VideoClip(0,1))  + wxPoint(10,0)); // Create a clip with length 0
        ASSERT_ZERO(VideoClip(0,1)->getLength());
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition);
        ShiftTrim(RightCenter(VideoClip(0,0)), Center(VideoClip(0,0))); // Caused crash in trim handling due to 'adjacent clip' having length 0
        Undo(3);
    }
    {
        StartTest("Clip to the left of the trim has length 0.");
        OpenPopupMenuAt(Center(VideoClip(0,0)));
        Type('i'); // fade &in
        ShiftTrim(RightCenter(VideoClip(0,1)), LeftCenter(VideoClip(0,1)) + wxPoint(-10,0)); // Create a clip with length 0
        ASSERT_ZERO(VideoClip(0,1)->getLength());
        ASSERT_VIDEOTRACK0(Transition)(VideoClip)(VideoClip);
        ShiftTrim(LeftCenter(VideoClip(0,2)), Center(VideoClip(0,2))); // Caused crash in trim handling due to 'adjacent clip' having length 0
        Undo(3);
    }
}

void TestBugs::testDeleteClipInbetweenTransitionsCausesTimelineMessUp()
{
    StartTestSuite();
    Zoom level(6);
    MakeInOutTransitionAfterClip t1(1);
    MakeInOutTransitionAfterClip t2(0);
    OpenPopupMenuAt(Center(VideoClip(0,2)));
    Type('t');
    ASSERT_EQUALS(VideoClip(0,1)->getLeftPts(), AudioClip(0,1)->getLeftPts());
    ASSERT_EQUALS(VideoTrack(0)->getLength(),AudioTrack(0)->getLength());
    ASSERT_EQUALS(VideoClip(0,4)->getRightPts(), AudioClip(0,4)->getRightPts());
    Undo();
}

} // namespace