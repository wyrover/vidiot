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

#include "TestTrimming.h"

#include "AudioClip.h"
#include "AudioTrack.h"
#include "CreateTransition.h"
#include "Cursor.h"
#include "ExecuteDrop.h"
#include "HelperApplication.h"
#include "HelperPlayback.h"
#include "HelperProjectView.h"
#include "HelperTimeline.h"
#include "HelperTimelineAssert.h"
#include "HelperTimelineDrag.h"
#include "HelperTimelinesView.h"
#include "HelperTimelineTrim.h"
#include "HelperTransition.h"
#include "HelperWindow.h"
#include "Timeline.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestTrimming::setUp()
{
    mProjectFixture.init();
}

void TestTrimming::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASESMove(
//////////////////////////////////////////////////////////////////////////

void TestTrimming::testSnapping()
{
    StartTestSuite();
    Zoom level(6);
    ConfigFixture.SnapToClips(true).SnapToCursor(true);
    PositionCursor(HCenter(VideoClip(0,2)));
    {
        StartTest("No snapping when dragged beyond snap distance");
        Trim(LeftCenter(VideoClip(0,2)),Center(VideoClip(0,2)) + wxPoint(gui::Layout::SnapDistance + 1,0),false);
        ASSERT_MORE_THAN(VideoClip(0,2)->getRightPts(), getTimeline().getCursor().getLogicalPosition());
        Undo();
    }
    {
        StartTest("Snap to cursor when inside snap distance");
        Trim(LeftCenter(VideoClip(0,2)),Center(VideoClip(0,2)) + wxPoint(gui::Layout::SnapDistance - 1,0),false);
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), getTimeline().getCursor().getLogicalPosition()); // due to snapping
        Undo();
    }
    {
        StartTest("Temporarily disable snapping");
        Trim(LeftCenter(VideoClip(0,2)),Center(VideoClip(0,2)) + wxPoint(gui::Layout::SnapDistance - 1,0),false,false);
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), getTimeline().getCursor().getLogicalPosition()); // due to snapping
        Type('d'); // disable snapping
        ASSERT_MORE_THAN(VideoClip(0,2)->getRightPts(), getTimeline().getCursor().getLogicalPosition());
        Type('d'); // enable snapping
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), getTimeline().getCursor().getLogicalPosition()); // due to snapping
        EndTrim(false);
        Undo();
    }

}

void TestTrimming::testKeyboardTrimming()
{
    StartTestSuite();
    Zoom level(3);
    
    auto TestBeginTrimSucceeds = [this](wxString title)
    {
        StartTest(title);
        PositionCursor(HCenter(VideoClip(0,2)));
        pts newlength = VideoClip(0,2)->getRightPts() - getTimeline().getCursor().getLogicalPosition();
        Type('b');
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), newlength);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
        ASSERT_EQUALS(getTimeline().getCursor().getLogicalPosition(), VideoClip(0,2)->getLeftPts());
        Undo();
    };

    auto TestEndTrimSucceeds = [this](wxString title)
    {
        StartTest(title);
        PositionCursor(HCenter(VideoClip(0,2)));
        pts newlength = getTimeline().getCursor().getLogicalPosition() - VideoClip(0,2)->getLeftPts();
        Type('e');
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), newlength);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
        ASSERT_EQUALS(getTimeline().getCursor().getLogicalPosition(), VideoClip(0,2)->getRightPts());
        Undo();
    };

    auto TestBeginTrimImpossible = [this](wxString title)
    {
        StartTest(title);
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::ExecuteDrop>();
        PositionCursor(HCenter(VideoClip(0,2)));
        Type('b');
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::ExecuteDrop>();
    };

    auto TestEndTrimImpossible = [this](wxString title)
    {
        StartTest(title);
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::ExecuteDrop>();
        PositionCursor(HCenter(VideoClip(0,2)));
        Type('e');
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::ExecuteDrop>();
    };

    TestBeginTrimSucceeds("Without other tracks: Begin trim");
    TestEndTrimSucceeds("Without other tracks: End trim");
    {
        StartTest("No change when cursor is on cut");
        ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();
        PositionCursor(LeftPixel(VideoClip(0,2)));
        Type('b');
        Type('e');
        ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();
    }
    
    triggerMenu(ID_ADDVIDEOTRACK);
    triggerMenu(ID_ADDAUDIOTRACK);
    TrimRight(VideoClip(0,4), - 250); // Make smaller for easier positioning
    
    TestBeginTrimSucceeds("With other track without clips: Begin trim");
    TestEndTrimSucceeds("With other track without clips: End trim");
    
    DragToTrack(1,VideoClip(0,4),AudioClip(0,4));
    TestBeginTrimSucceeds("With other track with empty clip: Begin trim");
    TestEndTrimSucceeds("With other track with empty clip: End trim");

    Drag(From(Center(VideoClip(1,1))).To(wxPoint(HCenter(VideoClip(0,2)), VCenter(VideoTrack(1)))));
    TestBeginTrimImpossible("With other track with fully obscuring non-empty clip: Begin trim");
    TestEndTrimImpossible("With other track with fully obscuring non-empty clip: End trim");
    Undo();

    Drag(From(Center(VideoClip(1,1))).AlignLeft(RightPixel(VideoClip(0,2)) - 20));
    TestBeginTrimSucceeds("With other track with partially right obscuring non-empty clip: Begin trim");
    TestEndTrimImpossible("With other track with partially right obscuring non-empty clip: End trim (no change)");
    Undo();
    
    Drag(From(RightCenter(VideoClip(1,1)) + wxPoint(-20,0)).AlignRight(LeftPixel(VideoClip(0,2)) + 20));
    TestBeginTrimImpossible("With other track with partially left obscuring non-empty clip: Begin trim");
    TestEndTrimSucceeds("With other track with partially left obscuring non-empty clip: End trim (no change)");
    Undo();
}

void TestTrimming::testKeyboardTrimmingDuringPlayback()
{
    StartTestSuite();
    StartTest("Start playback");
    PositionCursor(HCenter(VideoClip(0,4)));
    WaitForPlaybackStarted started;
    Type(' ');
    started.wait();

    StartTest("Trigger trim");
    WaitForPlaybackStopped stopped;
    WaitForPlaybackStarted startedAgain;
    pause(200);
    Type('b');
    stopped.wait();
    startedAgain.wait();

    ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
    ASSERT_LESS_THAN(VideoClip(0,4)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,4));
    ASSERT_EQUALS(VideoClip(0,5)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,5));

    StartTest("Stop playback");
    WaitForPlaybackStopped stoppedAgain;
    Type(' ');
    stoppedAgain.wait();
    pause(500);

    //ASSERT(getTimeline().getPlayer()->
    waitForIdle();
}



} // namespace