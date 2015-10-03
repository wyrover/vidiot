// Copyright 2013-2015 Eric Raijmakers.
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
    TimelineZoomIn(4);
    ConfigFixture.SnapToClips(true).SnapToCursor(true);
    TimelinePositionCursor(HCenter(VideoClip(0,2)));
    {
        StartTest("No snapping when dragged beyond snap distance");
        TimelineTrim(LeftCenter(VideoClip(0,2)),Center(VideoClip(0,2)) + wxPoint(gui::timeline::Timeline::SnapDistance + 1,0),false);
        ASSERT_MORE_THAN(VideoClip(0,2)->getRightPts(), getTimeline().getCursor().getLogicalPosition());
        Undo();
    }
    {
        StartTest("Snap to cursor when inside snap distance");
        TimelineTrim(LeftCenter(VideoClip(0,2)),Center(VideoClip(0,2)) + wxPoint(gui::timeline::Timeline::SnapDistance - 1,0),false);
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), getTimeline().getCursor().getLogicalPosition()); // due to snapping
        Undo();
    }
    {
        StartTest("Temporarily disable snapping");
        TimelineTrim(LeftCenter(VideoClip(0,2)),Center(VideoClip(0,2)) + wxPoint(gui::timeline::Timeline::SnapDistance - 1,0),false,false);
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), getTimeline().getCursor().getLogicalPosition()); // due to snapping
        TimelineKeyPress('d'); // disable snapping
        ASSERT_MORE_THAN(VideoClip(0,2)->getRightPts(), getTimeline().getCursor().getLogicalPosition());
        TimelineKeyPress('d'); // enable snapping
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), getTimeline().getCursor().getLogicalPosition()); // due to snapping
        TimelineEndTrim(false);
        Undo();
    }

}

void TestTrimming::testKeyboardTrimming()
{
    StartTestSuite();
    TimelineZoomIn(3);

    auto TestBeginTrimSucceeds = [this](wxString title)
    {
        StartTest(title);
        TimelinePositionCursor(HCenter(VideoClip(0,2)));
        pts newlength = VideoClip(0,2)->getRightPts() - getTimeline().getCursor().getLogicalPosition();
        TimelineKeyPress('b');
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), newlength);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
        ASSERT_EQUALS(getTimeline().getCursor().getLogicalPosition(), VideoClip(0,2)->getLeftPts());
        Undo();
    };

    auto TestEndTrimSucceeds = [this](wxString title)
    {
        StartTest(title);
        TimelinePositionCursor(HCenter(VideoClip(0,2)));
        pts newlength = getTimeline().getCursor().getLogicalPosition() - VideoClip(0,2)->getLeftPts();
        TimelineKeyPress('e');
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
        TimelinePositionCursor(HCenter(VideoClip(0,2)));
        TimelineKeyPress('b');
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::ExecuteDrop>();
    };

    auto TestEndTrimImpossible = [this](wxString title)
    {
        StartTest(title);
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::ExecuteDrop>();
        TimelinePositionCursor(HCenter(VideoClip(0,2)));
        TimelineKeyPress('e');
        ASSERT_CURRENT_COMMAND_TYPE<gui::timeline::command::ExecuteDrop>();
    };

    TestBeginTrimSucceeds("Without other tracks: Begin trim");
    TestEndTrimSucceeds("Without other tracks: End trim");
    {
        StartTest("No change when cursor is on cut");
        ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();
        TimelinePositionCursor(LeftPixel(VideoClip(0,2)));
        TimelineKeyPress('b');
        TimelineKeyPress('e');
        ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();
    }

    WindowTriggerMenu(ID_ADDVIDEOTRACK);
    WindowTriggerMenu(ID_ADDAUDIOTRACK);
    TimelineTrimRight(VideoClip(0,4), - 250); // Make smaller for easier positioning

    TestBeginTrimSucceeds("With other track without clips: Begin trim");
    TestEndTrimSucceeds("With other track without clips: End trim");

    TimelineDragToTrack(1,VideoClip(0,4),AudioClip(0,4));
    TestBeginTrimSucceeds("With other track with empty clip: Begin trim");
    TestEndTrimSucceeds("With other track with empty clip: End trim");

    TimelineDrag(From(Center(VideoClip(1,1))).To(wxPoint(HCenter(VideoClip(0,2)), VCenter(VideoTrack(1)))));
    TestBeginTrimImpossible("With other track with fully obscuring non-empty clip: Begin trim");
    TestEndTrimImpossible("With other track with fully obscuring non-empty clip: End trim");
    Undo();

    TimelineDrag(From(Center(VideoClip(1,1))).AlignLeft(RightPixel(VideoClip(0,2)) - 20));
    TestBeginTrimSucceeds("With other track with partially right obscuring non-empty clip: Begin trim");
    TestEndTrimImpossible("With other track with partially right obscuring non-empty clip: End trim (no change)");
    Undo();

    TimelineDrag(From(RightCenter(VideoClip(1,1)) + wxPoint(-20,0)).AlignRight(LeftPixel(VideoClip(0,2)) + 20));
    TestBeginTrimImpossible("With other track with partially left obscuring non-empty clip: Begin trim");
    TestEndTrimSucceeds("With other track with partially left obscuring non-empty clip: End trim (no change)");
    Undo();
}

void TestTrimming::testKeyboardTrimmingDuringPlayback()
{
    StartTestSuite();
    StartTest("Start playback");
    TimelinePositionCursor(HCenter(VideoClip(0,4)));
    WaitForPlaybackStarted started;
    TimelineKeyPress(' ');
    started.wait();

    StartTest("Trigger trim");
    WaitForPlaybackStopped stopped;
    WaitForPlaybackStarted startedAgain;
    pause(200);
    TimelineKeyPress('b');
    stopped.wait();
    startedAgain.wait();

    ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
    ASSERT_LESS_THAN(VideoClip(0,4)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,4));
    ASSERT_EQUALS(VideoClip(0,5)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,5));

    StartTest("Stop playback");
    WaitForPlaybackStopped stoppedAgain;
    TimelineKeyPress(' ');
    stoppedAgain.wait();
    pause(500);

    WaitForIdle;
}

void TestTrimming::testTrimmingUnlinkedClip()
{
    StartTestSuite();
    StartTest("Unlink");
    Unlink(VideoClip(0, 2));
    {
        StartTest("Trim unlinked video clip");
        TimelineTrimLeft(VideoClip(0, 2), +20, true); // Doesn't work due to the audio clip
        TimelineTrimRight(VideoClip(0, 2), -20, true); // Doesn't work due to the audio clip
        ASSERT_EQUALS(VideoClip(0, 2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0, 2));
        TimelineTrimLeft(VideoClip(0, 2), +20, false);
        ASSERT_LESS_THAN(VideoClip(0, 2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0, 2));
        Undo();
        ASSERT_EQUALS(VideoClip(0, 2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0, 2));
        TimelineTrimRight(VideoClip(0, 2), -20, false);
        ASSERT_LESS_THAN(VideoClip(0, 2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0, 2));
        Undo();
    }
    {
        StartTest("Trim unlinked audio clip");
        TimelineTrimLeft(AudioClip(0, 2), +20, true); // Doesn't work due to the video clip
        TimelineTrimRight(AudioClip(0, 2), -20, true); // Doesn't work due to the video clip
        ASSERT_EQUALS(AudioClip(0, 2)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0, 2));
        TimelineTrimLeft(AudioClip(0, 2), +20, false);
        ASSERT_LESS_THAN(AudioClip(0, 2)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0, 2));
        Undo();
        ASSERT_EQUALS(AudioClip(0, 2)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0, 2));
        TimelineTrimRight(AudioClip(0, 2), -20, false);
        ASSERT_LESS_THAN(AudioClip(0, 2)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0, 2));
        Undo();
    }
    {
        StartTest("Shift trim and extend unlinked video clip");
        TimelineDeleteClip(AudioClip(0,2));
        TimelineTrimRight(VideoClip(0, 2), +200, true); // Not possible: already at max length
        ASSERT_EQUALS(VideoClip(0, 2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0, 2));
        Undo();
    }
    {
        StartTest("Shift trim and extend unlinked audio clip");
        TimelineDeleteClip(VideoClip(0,2));
        TimelineTrimRight(AudioClip(0, 2), +200, true); // Not possible: already at max length
        ASSERT_EQUALS(AudioClip(0, 2)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0, 2));
        Undo();
    }
}

void TestTrimming::testTrimmingWithoutTrimmingLink()
{
    StartTestSuite();
    TimelineZoomIn(5);
    TimelineTrimRight(VideoClip(0,1), -100, false);
    TimelineTrimLeft(VideoClip(0,1), 100, false);
    ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(EmptyClip)(VideoClip);
    ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip)(EmptyClip)(AudioClip);
    {
        StartTest("Video begin enlarge");
        TimelineTrimLeft(VideoClip(0,2), -50, false, false);
        TimelineKeyPress('u');
        TimelineEndTrim(false);
        ASSERT_LESS_THAN(VideoClip(0, 2)->getLeftPts(), AudioClip(0, 2)->getLeftPts());
        ASSERT_MORE_THAN(VideoClip(0, 2)->getLength(), AudioClip(0, 2)->getLength());
        ASSERT_EQUALS(VideoClip(0, 2)->getRightPts(), AudioClip(0, 2)->getRightPts());
        ASSERT_EQUALS(VideoClip(0, 4)->getLeftPts(), mProjectFixture.OriginalPtsOfVideoClip(0, 2));
        ASSERT_EQUALS(AudioClip(0, 4)->getLeftPts(), mProjectFixture.OriginalPtsOfAudioClip(0, 2));
        Undo();
    }
    {
        StartTest("Audio begin enlarge");
        TimelineTrimLeft(AudioClip(0,2), -50, false, false);
        TimelineKeyPress('u');
        TimelineEndTrim(false);
        ASSERT_LESS_THAN(AudioClip(0, 2)->getLeftPts(), VideoClip(0, 2)->getLeftPts());
        ASSERT_MORE_THAN(AudioClip(0, 2)->getLength(), VideoClip(0, 2)->getLength());
        ASSERT_EQUALS(AudioClip(0, 2)->getRightPts(), VideoClip(0, 2)->getRightPts());
        ASSERT_EQUALS(AudioClip(0, 4)->getLeftPts(), mProjectFixture.OriginalPtsOfAudioClip(0, 2));
        ASSERT_EQUALS(VideoClip(0, 4)->getLeftPts(), mProjectFixture.OriginalPtsOfVideoClip(0, 2));
        Undo();
    }
    {
        StartTest("Video begin shrink");
        TimelineTrimLeft(VideoClip(0,2), 50, false, false);
        TimelineKeyPress('u');
        TimelineEndTrim(false);
        ASSERT_MORE_THAN(VideoClip(0, 2)->getLeftPts(), AudioClip(0, 2)->getLeftPts());
        ASSERT_LESS_THAN(VideoClip(0, 2)->getLength(), AudioClip(0, 2)->getLength());
        ASSERT_EQUALS(VideoClip(0, 2)->getRightPts(), AudioClip(0, 2)->getRightPts());
        ASSERT_EQUALS(VideoClip(0, 4)->getLeftPts(), mProjectFixture.OriginalPtsOfVideoClip(0, 2));
        ASSERT_EQUALS(AudioClip(0, 4)->getLeftPts(), mProjectFixture.OriginalPtsOfAudioClip(0, 2));
        Undo();
    }
    {
        StartTest("Audio begin shrink");
        TimelineTrimLeft(AudioClip(0,2), 50, false, false);
        TimelineKeyPress('u');
        TimelineEndTrim(false);
        ASSERT_MORE_THAN(AudioClip(0, 2)->getLeftPts(), VideoClip(0, 2)->getLeftPts());
        ASSERT_LESS_THAN(AudioClip(0, 2)->getLength(), VideoClip(0, 2)->getLength());
        ASSERT_EQUALS(AudioClip(0, 2)->getRightPts(), VideoClip(0, 2)->getRightPts());
        ASSERT_EQUALS(AudioClip(0, 4)->getLeftPts(), mProjectFixture.OriginalPtsOfAudioClip(0, 2));
        ASSERT_EQUALS(VideoClip(0, 4)->getLeftPts(), mProjectFixture.OriginalPtsOfVideoClip(0, 2));
        Undo();
    }
    {
        StartTest("Video end enlarge");
        TimelineTrimRight(VideoClip(0,2), 50, false, false);
        TimelineKeyPress('u');
        TimelineEndTrim(false);
        ASSERT_MORE_THAN(VideoClip(0, 2)->getRightPts(), AudioClip(0, 2)->getRightPts());
        ASSERT_MORE_THAN(VideoClip(0, 2)->getLength(), AudioClip(0, 2)->getLength());
        ASSERT_EQUALS(VideoClip(0, 2)->getLeftPts(), AudioClip(0, 2)->getLeftPts());
        ASSERT_EQUALS(VideoClip(0, 4)->getLeftPts(), mProjectFixture.OriginalPtsOfVideoClip(0, 2));
        ASSERT_EQUALS(AudioClip(0, 4)->getLeftPts(), mProjectFixture.OriginalPtsOfAudioClip(0, 2));
        Undo();
    }
    {
        StartTest("Audio end enlarge");
        TimelineTrimRight(AudioClip(0,2), 50, false, false);
        TimelineKeyPress('u');
        TimelineEndTrim(false);
        ASSERT_MORE_THAN(AudioClip(0, 2)->getRightPts(), VideoClip(0, 2)->getRightPts());
        ASSERT_MORE_THAN(AudioClip(0, 2)->getLength(), VideoClip(0, 2)->getLength());
        ASSERT_EQUALS(AudioClip(0, 2)->getLeftPts(), VideoClip(0, 2)->getLeftPts());
        ASSERT_EQUALS(AudioClip(0, 4)->getLeftPts(), mProjectFixture.OriginalPtsOfAudioClip(0, 2));
        ASSERT_EQUALS(VideoClip(0, 4)->getLeftPts(), mProjectFixture.OriginalPtsOfVideoClip(0, 2));
        Undo();
    }
    {
        StartTest("Video end shrink");
        TimelineTrimRight(VideoClip(0,2), -50, false, false);
        TimelineKeyPress('u');
        TimelineEndTrim(false);
        ASSERT_LESS_THAN(VideoClip(0, 2)->getRightPts(), AudioClip(0, 2)->getRightPts());
        ASSERT_LESS_THAN(VideoClip(0, 2)->getLength(), AudioClip(0, 2)->getLength());
        ASSERT_EQUALS(VideoClip(0, 2)->getLeftPts(), AudioClip(0, 2)->getLeftPts());
        ASSERT_EQUALS(VideoClip(0, 4)->getLeftPts(), mProjectFixture.OriginalPtsOfVideoClip(0, 2));
        ASSERT_EQUALS(AudioClip(0, 4)->getLeftPts(), mProjectFixture.OriginalPtsOfAudioClip(0, 2));
        Undo();
    }
    {
        StartTest("Audio end shrink");
        TimelineTrimRight(AudioClip(0,2), -50, false, false);
        TimelineKeyPress('u');
        TimelineEndTrim(false);
        ASSERT_LESS_THAN(AudioClip(0, 2)->getRightPts(), VideoClip(0, 2)->getRightPts());
        ASSERT_LESS_THAN(AudioClip(0, 2)->getLength(), VideoClip(0, 2)->getLength());
        ASSERT_EQUALS(AudioClip(0, 2)->getLeftPts(), VideoClip(0, 2)->getLeftPts());
        ASSERT_EQUALS(AudioClip(0, 4)->getLeftPts(), mProjectFixture.OriginalPtsOfAudioClip(0, 2));
        ASSERT_EQUALS(VideoClip(0, 4)->getLeftPts(), mProjectFixture.OriginalPtsOfVideoClip(0, 2));
        Undo();
    }
}


} // namespace
