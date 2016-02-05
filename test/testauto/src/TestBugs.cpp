// Copyright 2013-2016 Eric Raijmakers.
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

#include "UtilInitAvcodec.h"

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
    TimelineTrimRight(VideoClip(0,3),4);
    TimelineTrimRight(VideoClip(0,3),10);
    TimelineTrimRight(VideoClip(0,3),15);
    TimelineTrimRight(VideoClip(0,3),20);
}

void TestBugs::testHangupAfterResettingDetailsView()
{
    StartTestSuite();
    TimelineLeftClick(VTopQuarterHCenter(VideoClip(0,2)));
    TimelineSelectClips({});
}

void TestBugs::testDetailsNotShownAfterMovingTimelineCursor()
{
    StartTestSuite();
    TimelineLeftClick(Center(VideoClip(0,3)));
    TimelinePositionCursor(HCenter(VideoClip(0,3)));
    TimelineLeftClick(Center(VideoClip(0,3)));
    ASSERT(DetailsView(VideoClip(0,3)));
}

void TestBugs::testLinkingErrorWhenDroppingOverBeginOfLinkedClip()
{
    StartTestSuite();
    WindowTriggerMenu(ID_ADDVIDEOTRACK);
    TimelineTrimLeft(VideoClip(0,4),40,false);
    TimelineDrag(From(Center(VideoClip(0,6))).To(wxPoint(RightPixel(VideoClip(0,4)),VCenter(getSequence()->getVideoTrack(1)))));
    ASSERT_EQUALS(VideoClip(0,5)->getLink(),AudioClip(0,6));
    TimelineTrimLeft(VideoClip(0,5),10,false); // This caused an assert, because there was a problem with this clip (video(0,5)) link.
}

void TestBugs::testErrorInGetNextHandlingForEmptyClips()
{
    StartTestSuite();
    WindowTriggerMenu(ID_ADDVIDEOTRACK);
    WindowTriggerMenu(ID_ADDVIDEOTRACK);

    TimelineDragToTrack(1,VideoClip(0,5),model::IClipPtr());
    TimelineDrag(From(Center(VideoClip(1,1))).To(wxPoint(HCenter(VideoClip(0,4)),VCenter(VideoClip(1,1)))));

    TimelineDragToTrack(2,VideoClip(0,6),model::IClipPtr());
    TimelineDrag(From(Center(VideoClip(2,1))).AlignLeft(LeftPixel(VideoClip(1,1))));

    TimelineLeftClick(Center(VideoClip(1,1)));
    ASSERT(DetailsView(VideoClip(1,1)));

    SetValue(DetailsClipView()->getScalingSlider(), 4000); // Same as pressing 6 * PageUp
    SetValue(DetailsClipView()->getPositionXSlider(), 424); // Same as pressing 4 * PageDown
    SetValue(DetailsClipView()->getPositionYSlider(), 256); // Same as pressing 4 * PageDown

    TimelineLeftClick(Center(VideoClip(2,1)));
    ASSERT(DetailsView(VideoClip(2,1)));

    SetValue(DetailsClipView()->getScalingSlider(), 4000); // Same as pressing 6 * PageUp
    SetValue(DetailsClipView()->getPositionXSlider(), 304); // Same as pressing 4 * PageUp
    SetValue(DetailsClipView()->getPositionYSlider(), 176); // Same as pressing 4 * PageUp

    TimelinePositionCursor(LeftPixel(VideoClip(2,1)) - 5);

    Play(LeftPixel(VideoClip(2,1)) - 3,2500); // Start before the clip, in the empty area. Due to a bug in 'getNext' handling for empty clips the clips after the empty area were not shown, or too late.

    Undo(9);
}

void TestBugs::testDraggingWithoutSelection()
{
    StartTestSuite();

    TimelineLeftClick(Center(VideoClip(0,1))); // Select the clip
    TimelineKeyDown(WXK_CONTROL);
    TimelineLeftDown(); // Deselects the clip already
    TimelineMove(Center(VideoClip(0,3))); // Starts the drag without anything being selected: ASSERT at the time of the bug. Now the drag should be simply omitted.
    ASSERT_HISTORY_END(cmd::ProjectViewCreateSequence);
    TimelineKeyUp(WXK_CONTROL);
    TimelineLeftUp();
}

void TestBugs::testBugsWithLongTimeline()
{
    StartTestSuite();
    model::SequencePtr sequence = getSequence();
    Config::setShowDebugInfo(true);
    WindowTriggerMenu(ID_CLOSESEQUENCE);

    ExtendSequenceWithRepeatedClips(sequence, mProjectFixture.InputFiles, 30);

    ProjectViewOpenTimelineForSequence(sequence);

    TimelineZoomIn(4);
    TimelineKeyPress(WXK_END); // Move scrollbars to end

    util::thread::RunInMainAndWait([]
    {
        // Workaround for Windows 7 bug: http://trac.wxwidgets.org/ticket/14329 / http://trac.wxwidgets.org/ticket/14313
        gui::Window::get().SetSize(gui::Window::get().GetSize());
    });

    {
        StartTest("Bug: Audio playback continued, video playback stopped");
        Play(HCenter(VideoClip(0,-1)), 2000);
    }
    {
        StartTest("Bug: Playback did not stop after pressing shift");
        TimelinePositionCursor(HCenter(VideoClip(0,-2)));
        // NOTE: Don't use WaitForIdle() when the video is playing!!!
        //       When the video is playing, the system does not become Idle (playback events).
        WaitForPlayback started(true);
        WaitForPlayback stopped(false);
        TimelineKeyPress(' ');
        started.wait();
        pause(1000);
        TimelineKeyDown(WXK_SHIFT);
        pause(1000);
        TimelineKeyUp(WXK_SHIFT);
        pause(1000);
        TimelineKeyPress(' ');
        stopped.wait();
        TimelineLeftClick(Center(VideoClip(0,-2)));
        TimelineKeyPress(WXK_DELETE);
        ASSERT(VideoClip(0,-2)->isA<model::EmptyClip>());
    }
    {
        StartTest("Bug: StackOverflow when saving and when loading");
        sequence.reset();
        DirAndFile tempDir_fileName = mProjectFixture.saveAndReload();
    }
    CloseProjectAndAvoidSaveDialog();
    Config::setShowDebugInfo(false);
}

void TestBugs::testPlaybackEmptyClip()
{
    StartTestSuite();

    TimelineLeftClick(Center(VideoClip(0,3)));
    TimelineKeyPress(WXK_DELETE);
    TimelinePositionCursor(HCenter(VideoClip(0,3)));
    util::thread::RunInMainAndWait([]
    {
        model::VideoFramePtr frame = boost::dynamic_pointer_cast<model::EmptyClip>(VideoClip(0, 3))->getNextVideo(model::VideoCompositionParameters().setBoundingBox(wxSize(100, 100)));
        ASSERT_NONZERO(frame);
        model::AudioChunkPtr chunk = boost::dynamic_pointer_cast<model::EmptyClip>(AudioClip(0, 3))->getNextAudio(model::AudioCompositionParameters().setPts(AudioClip(0, 3)->getLeftPts()).determineChunkSize());
        ASSERT_NONZERO(chunk);
    });
    // Note: do not pause() or press space at this point. The getNexts above 'mess up' the administration (audioclip is already at end, but the cursor position is not)
    Play(RightPixel(VideoClip(0,3)) - 3,2000);
}

void TestBugs::testTrimmingClipAdjacentToZeroLengthClipUsedForTransition()
{
    StartTestSuite();
    TimelineZoomIn(6);
    {
        StartTest("Clip to the right of the trim has length 0.");
        TimelineLeftClick(Center(VideoClip(0,1)));
        TimelineKeyPress('o'); // fade &out
        TimelineShiftTrim(LeftCenter(VideoClip(0,1)), RightCenter(VideoClip(0,1))  + wxPoint(10,0)); // Create a clip with length 0
        ASSERT_ZERO(VideoClip(0,1)->getLength());
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition);
        TimelineShiftTrim(RightCenter(VideoClip(0,0)), Center(VideoClip(0,0))); // Caused crash in trim handling due to 'adjacent clip' having length 0
        Undo(3);
    }
    {
        StartTest("Clip to the left of the trim has length 0.");
        TimelineLeftClick(Center(VideoClip(0,0)));
        TimelineKeyPress('i'); // fade &in
        TimelineShiftTrim(RightCenter(VideoClip(0,1)), LeftCenter(VideoClip(0,1)) + wxPoint(-10,0)); // Create a clip with length 0
        ASSERT_ZERO(VideoClip(0,1)->getLength());
        ASSERT_VIDEOTRACK0(Transition)(VideoClip)(VideoClip);
        TimelineShiftTrim(LeftCenter(VideoClip(0,2)), Center(VideoClip(0,2))); // Caused crash in trim handling due to 'adjacent clip' having length 0
        Undo(3);
    }
}

void TestBugs::testDeleteClipInbetweenTransitionsCausesTimelineMessUp()
{
    StartTestSuite();
    TimelineZoomIn(6);
    MakeInOutTransitionAfterClip t1(1);
    MakeInOutTransitionAfterClip t2(0);
    TimelineLeftClick(Center(VideoClip(0,2)));
    TimelineKeyDown(WXK_SHIFT);
    TimelineKeyPress(WXK_DELETE);
    TimelineKeyUp(WXK_SHIFT);
    ASSERT_EQUALS(VideoClip(0,1)->getLeftPts(), AudioClip(0,1)->getLeftPts());
    ASSERT_EQUALS(VideoTrack(0)->getLength(),AudioTrack(0)->getLength());
    ASSERT_EQUALS(VideoClip(0,4)->getRightPts(), AudioClip(0,4)->getRightPts());
    Undo();
}

void TestBugs::testTrimClipInbetweenTransitionsCausesCrash()
{
    StartTestSuite();
    TimelineZoomIn(3);
    MakeInOutTransitionAfterClip t1(1,true);
    t1.dontUndo();
    MakeInOutTransitionAfterClip t2(0,true);
    t2.dontUndo();
    TimelineZoomIn(2);
    // Project saved and read again to avoid a zoom operation having
    // occurred on the timeline. In the 'buggy' case, a variable was not
    // properly initialized - which was undone by zooming which caused an
    // update of that variable.
    DirAndFile tempDir_fileName = mProjectFixture.saveAndReload();
    util::thread::RunInMainAndWait([]()
    {
        getTimeline().getScrolling().align(50, 2);
    });
    TimelinePositionCursor(HCenter(VideoClip(0, 1)));
    TimelineKeyPress('e');
    Undo();
}

void TestBugs::testCrashWhenDroppingPartiallyOverATransition()
{
    StartTestSuite();
    TimelineZoomIn(4);
    {
        MakeInOutTransitionAfterClip preparation(1,true);
        {
            StartTest("AudioTransition: Drop a clip with its left edge exactly on the middle of a transition");
            TimelineDrag(From(Center(AudioClip(0,4))).AlignLeft(HCenter(AudioClip(0,2))));
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(EmptyClip);
            ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(EmptyClip);
            ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
            ASSERT_EQUALS(AudioClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,3));
            Undo();
        }
        {
            StartTest("AudioTransition: Drop a clip with its left edge inside the right half of a transition");
            TimelineDrag(From(Center(AudioClip(0,4))).AlignLeft(HCenter(AudioClip(0,2)) + 5));
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(EmptyClip);
            ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip)(EmptyClip);
            ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
            ASSERT_EQUALS(AudioClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,3));
            Undo();
        }
        {
            StartTest("AudioTransition: Drop a clip with its right edge inside the left half of a transition");
            TimelineDrag(From(Center(AudioClip(0,4))).AlignRight(HCenter(AudioClip(0,2)) - 5));
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(EmptyClip);
            ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip)(EmptyClip);
            ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
            ASSERT_EQUALS(AudioClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,3));
            Undo();
        }
    }
    {
        MakeInOutTransitionAfterClip preparation(1);
        {
            StartTest("VideoTransition: Drop a clip with its left edge exactly on the middle of a transition");
            TimelineDrag(From(Center(VideoClip(0,4))).AlignLeft(HCenter(VideoClip(0,2))));
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(EmptyClip);
            ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(EmptyClip);
            ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
            ASSERT_EQUALS(AudioClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,3));
            Undo();
        }
        {
            StartTest("VideoTransition: Drop a clip with its left edge inside the right half of a transition");
            TimelineDrag(From(Center(VideoClip(0,4))).AlignLeft(HCenter(VideoClip(0,2)) + 5));
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(EmptyClip);
            ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip)(EmptyClip);
            ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
            ASSERT_EQUALS(AudioClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,3));
            Undo();
        }
        {
            StartTest("VideoTransition: Drop a clip with its right edge inside the left half of a transition");
            TimelineDrag(From(Center(VideoClip(0,4))).AlignRight(HCenter(VideoClip(0,2)) - 5));
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(EmptyClip);
            ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip)(EmptyClip);
            ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
            ASSERT_EQUALS(AudioClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,3));
            Undo();
        }
    }
}

void TestBugs::testShiftTrimNotAllowedWithAdjacentClipInOtherTrack()
{
    StartTestSuite();
    TimelineZoomIn(2);
    WindowTriggerMenu(ID_ADDVIDEOTRACK);
    WindowTriggerMenu(ID_ADDAUDIOTRACK);
    TimelineDragToTrack(1,VideoClip(0,2),AudioClip(0,2));
    ASSERT_VIDEOTRACK1(     EmptyClip      )(VideoClip);
    ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(EmptyClip);
    ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(EmptyClip);
    ASSERT_AUDIOTRACK1(     EmptyClip      )(AudioClip);
    {
        StartTest("TimelineEndTrim allowed even with adjacent touching clip in other track");
        TimelineTrimRight(VideoClip(0,1), -20);
        ASSERT_LESS_THAN(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        Undo();
    }
    {
        StartTest("TimelineEndTrim allowed even with adjacent touching clip in other track");
        TimelineTrimLeft(VideoClip(0,1), 20);
        ASSERT_LESS_THAN(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        Undo();
    }
}

void TestBugs::testAddNonexistentFileViaRedo()
{
    StartTestSuite();
    WindowTriggerMenu(ID_CLOSESEQUENCE);

    // Create temp dir with temp file
    RandomTempDirPtr tempDir = boost::make_shared<RandomTempDir>();
    wxFileName filepath(tempDir->getFileName().GetLongPath(),"file","avi");
    bool copyok = wxCopyFile( getListOfInputPaths().front()->getPath().GetLongPath(), filepath.GetLongPath(), false );
    ASSERT(copyok);

    // Add folder to project view
    wxString sFolder1( "Folder1" );
    model::FolderPtr folder1 = ProjectViewAddFolder( sFolder1 );

    // Add file to folder
    model::Files files1 = ProjectViewAddFiles({ filepath }, folder1);
    ASSERT_WATCHED_PATHS_COUNT(2);

    // Remove the file via 'Undo'
    Undo();
    ASSERT_WATCHED_PATHS_COUNT(1);

    // Remove the file from disk
    tempDir.reset();

    // Add the file again (via 'Redo')
    gui::Dialog::get().setConfirmation(); // Trigger ok when the application informs that the file was deleted.
    Redo();
    ASSERT_WATCHED_PATHS_COUNT(1);

    // Now make the sequence (is empty)
    model::SequencePtr sequence1 = ProjectViewCreateSequence( folder1 );
    ASSERT_ZERO(VideoTrack(0)->getLength());
    ASSERT_ZERO(AudioTrack(0)->getLength());
}

void TestBugs::testAddNonexistentFileViaUndo()
{
    StartTestSuite();
    WindowTriggerMenu(ID_CLOSESEQUENCE);

    // Create temp dir with temp file
    RandomTempDirPtr tempDir = boost::make_shared<RandomTempDir>();
    model::IPaths inputfiles = getListOfInputPaths();
    wxString aviFileName = inputfiles.front()->getPath().GetLongPath();
    wxFileName filepath(tempDir->getFileName().GetLongPath(),"file","avi");
    bool copyok = wxCopyFile( aviFileName, filepath.GetLongPath(), false );
    ASSERT(copyok);

    // Add folder to project view
    wxString sFolder1( "Folder1" );
    model::FolderPtr folder1 = ProjectViewAddFolder( sFolder1 );

    // Add file to folder
    model::Files files1 = ProjectViewAddFiles({ filepath }, folder1);
    ASSERT_WATCHED_PATHS_COUNT(2);

    // Remove the file via 'Delete'
    ProjectViewRemove(files1.front());
    ASSERT_WATCHED_PATHS_COUNT(1);

    // Remove the file from disk
    tempDir.reset();

    // Add the file again (via 'Undo')
    gui::Dialog::get().setConfirmation(); // Trigger ok when the application informs that the file was deleted.
    Undo();
    ASSERT_WATCHED_PATHS_COUNT(1);

    // Now make the sequence (is empty)
    model::SequencePtr sequence1 = ProjectViewCreateSequence( folder1 );
    ASSERT_ZERO(VideoTrack(0)->getLength());
    ASSERT_ZERO(AudioTrack(0)->getLength());
}

void TestBugs::testPlaybackWithMultipleAudioTracks()
{
    StartTestSuite();

    WindowTriggerMenu(ID_ADDAUDIOTRACK);
    TimelineDragToTrack(1,model::IClipPtr(),AudioClip(0,2));
    Play(HCenter(AudioClip(0,0)), 1000);
}

void TestBugs::testCrashOnShiftDeleteWithMultipleTracks()
{
    StartTestSuite();
    {
        StartTest("Second video track empty");
        WindowTriggerMenu(ID_ADDVIDEOTRACK);
        ASSERT_MORE_THAN(VideoClip(0,5)->getLeftPts(), VideoTrack(1)->getLength());
        TimelineShiftDeleteClip(VideoClip(0,5));
        ASSERT_EQUALS(VideoClip(0,5)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,6));
        Undo();
    }
    {
        StartTest("Second video track nonempty but shorter than track 1");
        TimelineDragToTrack(1,VideoClip(0,2),model::IClipPtr());
        ASSERT_VIDEOTRACK1(     EmptyClip      )(VideoClip);
        ASSERT_MORE_THAN(VideoClip(0,5)->getLeftPts(), VideoTrack(1)->getLength());
        TimelineShiftDeleteClip(VideoClip(0,5));
        ASSERT_EQUALS(VideoClip(0,5)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,6));
        Undo();
    }
    Undo(); // Add video track
    {
        StartTest("Second audio track empty");
        WindowTriggerMenu(ID_ADDAUDIOTRACK);
        ASSERT_MORE_THAN(AudioClip(0,5)->getLeftPts(), AudioTrack(1)->getLength());
        TimelineShiftDeleteClip(AudioClip(0,5));
        ASSERT_EQUALS(AudioClip(0,5)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,6));
        Undo();
    }
    {
        StartTest("Second audio track nonempty but shorter than track 1");
        TimelineDragToTrack(1,model::IClipPtr(),AudioClip(0,2));
        ASSERT_AUDIOTRACK1(     EmptyClip      )(AudioClip);
        ASSERT_MORE_THAN(AudioClip(0,5)->getLeftPts(), AudioTrack(1)->getLength());
        TimelineShiftDeleteClip(AudioClip(0,5));
        ASSERT_EQUALS(AudioClip(0,5)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,6));
        Undo();
    }
    Undo(); // Add audio track
}

void TestBugs::testCrashWhenTrimmingWithTransitionOnOneSideOfCut()
{
    StartTestSuite();
    TimelineZoomIn(3);
    {
        StartTest("Left edge");
        TimelineTrimRight(VideoClip(0,3), -20);
        TimelineMove(RightCenter(VideoClip(0,3)));
        TimelineKeyPress('n'); // Create the transition
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
        wxPoint position(LeftPixel(VideoClip(0,4)), VCenter(VideoClip(0,5)));
        StartTest("Left edge: Click");
        TimelineLeftClick(position); // Caused crash: click under the left edge of the transition
        StartTest("Left edge: Trim");
        TimelineTrim(position,position + wxPoint(20,0),false,true);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(VideoClip);
        Undo();
        StartTest("Left edge: Shift trim");
        TimelineTrim(position,position + wxPoint(20,0),true,true);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip);
        Undo(2);
    }
    {
        StartTest("Right edge");
        TimelineTrimLeft(VideoClip(0,3), 20);
        TimelineMove(LeftCenter(VideoClip(0,3)));
        TimelineKeyPress('p'); // Create the transition
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        wxPoint position(RightPixel(VideoClip(0,3)), VCenter(VideoClip(0,2)));
        StartTest("Right edge: Click");
        TimelineLeftClick(position); // Caused crash: click under the right edge of the transition
        StartTest("Right edge: Trim");
        TimelineTrim(position,position - wxPoint(20,0),false,true);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(VideoClip)(VideoClip);
        Undo();
        StartTest("Right edge: Shift trim");
        TimelineTrim(position,position - wxPoint(20,0),true,true);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        Undo(2);
    }
}

void TestBugs::testTrimmingWithTransitionOnOneSideOfCut()
{
    StartTestSuite();
    TimelineZoomIn(3);
    {
        StartTest("Left edge");
        TimelineTrimRight(VideoClip(0,1), -20);
        TimelineMove(RightCenter(VideoClip(0,1)));
        TimelineKeyPress('n'); // Create the transition
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        // Due to the 'rounding errors correction' in test::RightPixel(), RightPixel moves
        // left until the requested clip is returned. However, for this specific scenario
        // we want to get a pixel position that is beyond the given clip's 'rightmost'
        // position as returned by RightPixel. Hence, the +1.
        pixel right = RightPixel(VideoClip(0,1)) + 1;
        wxPoint position(right, VCenter(VideoClip(0,1)));
        ASSERT_EQUALS(getTimeline().getMouse().getInfo(position).logicalclipposition, ::gui::timeline::TransitionLeftClipEnd);
        pts length = VideoClip(0,1)->getLength();
        StartTest("Right edge: Trim");
        TimelineTrim(position,position - wxPoint(20,0),false,true);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(EmptyClip)(VideoClip);
        ASSERT_LESS_THAN(VideoClip(0,1)->getLength(),length);
        Undo();
        StartTest("Right edge: Shift trim");
        TimelineTrim(position,position - wxPoint(20,0),true,true);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_LESS_THAN(VideoClip(0,1)->getLength(),length);
        Undo(2);
    }
    {
        StartTest("Right edge");
        TimelineTrimLeft(VideoClip(0,1), 20);
        TimelineMove(LeftCenter(VideoClip(0,1)));
        TimelineKeyPress('p'); // Create the transition
        ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(VideoClip);
        // Due to the 'rounding errors correction' in test::LeftPixel(), LeftPixel moves
        // right until the requested clip is returned. However, for this specific scenario
        // we want to get a pixel position that is beyond the given clip's 'leftmost'
        // position as returned by LeftPixel. Hence, the -1.
        pixel left = LeftPixel(VideoClip(0,2)) - 1;
        wxPoint position(left, VCenter(VideoClip(0,1)));
        pts length = VideoClip(0,2)->getLength();
        StartTest("Left edge: Trim");
        TimelineTrim(position,position + wxPoint(20,0),false,true);
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(VideoClip);
        ASSERT_LESS_THAN(VideoClip(0,2)->getLength(),length);
        Undo();
        StartTest("Left edge: Shift trim");
        TimelineTrim(position,position + wxPoint(20,0),true,true);
        ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_LESS_THAN(VideoClip(0,2)->getLength(),length);
        Undo(2);
    }
}

void TestBugs::testSnapClipBeforeBeginOfTimeline()
{
    StartTestSuite();
    ConfigOverrule<bool> overruleSnapToCursor(Config::sPathTimelineSnapClips,false);
    ConfigOverrule<bool> overruleSnapToClips(Config::sPathTimelineSnapCursor,true);
    TimelineZoomIn(5);
    StartTest("Snap to cursor");
    ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip);
    TimelinePositionCursor(RightPixel(VideoClip(0,0)) - 1);
    TimelineDrag(From(Center(VideoClip(0,0))).To(Center(VideoClip(0,3))).DontReleaseMouse());
    TimelineDrag(To(Center(VideoClip(0,0))));
    ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0));
    ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
    ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
}


void TestBugs::testCrashWhenDeterminingClipSizeBoundsForLinkedClipsWithDifferentLengthAndOutTransition()
{
    StartTestSuite();
    TimelineZoomIn(3);
    StartTest("Preparation");
    // Create
    // (VideoClip)(VideoClip)
    // (        AudioClip       )
    Unlink(VideoClip(0,2));
    TimelineDrag(From(Center(VideoClip(0,2))).AlignLeft(HCenter(VideoClip(0,1))));
    TimelineTrimRight(VideoClip(0,2), -100, false); // Audio clip had to be longer that the two video clips
    StartTest("Creation of transition caused the crash.");
    // Audio clip must be selected, because the crash was caused by showing the clip details
    // directly after the transition was created.
    TimelineLeftClick(Center(AudioClip(0,1)));
    TimelineMove(RightCenter(AudioClip(0,1)));
    TimelineKeyPress('n');
    StartTest("Trimming caused the crash.");
    // Same problem, different scenario.
    // (VideoClip)(VideoClip)
    // (    AudioClip   )(Transition) - Transition by clicking on next audio clip and creating a 'fade from previous'
    // Trimming on the left side of one of the first clips caused the crash.
    TimelineTrimLeft(VideoClip(0,1), 20, false);
    Undo();
    TimelineTrimLeft(AudioClip(0,1), 20, false);
    Undo(5);
}

void TestBugs::testCrashWhenCreatingCrossfadeViaKeyboardTwice()
{
    StartTestSuite();
    TimelineZoomIn(5);
    StartTest("Prepare");
    TimelineMove(LeftCenter(VideoClip(0,1)));
    TimelineKeyPress('p'); // Create the transition
    ASSERT_HISTORY_END(cmd::ProjectViewCreateSequence)(cmd::Combiner);
    ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip);
    TimelineMove(RightCenter(VideoClip(0,1)) + wxPoint(3,0));
    StartTest("InTransition");
    TimelineKeyPress('p'); // Create transition again: not possible
    TimelineKeyPress('i'); // Create transition again: not possible
    TimelineMove(LeftCenter(VideoClip(0,1)) - wxPoint(3,0));
    StartTest("OutTransition");
    TimelineKeyPress('n'); // Create transition again: not possible
    TimelineKeyPress('o'); // Create transition again: not possible
    ASSERT_HISTORY_END(cmd::ProjectViewCreateSequence)(cmd::Combiner);
}

void TestBugs::testCrashCausedByCreatingTransitionAtAudioClipEndAfterReadingProjectFromDisk()
{
    StartTestSuite();
    TimelineZoomIn(3);
    DirAndFile tempDir_fileName = mProjectFixture.saveAndReload();
    TimelineLeftClick(Center(AudioClip(0,3)));
    // Transition created with keyboard press specifically.
    // 'MakeInOutTransitionAfterClip' causes the file object to be initialized before
    // creating the transition.
    TimelineKeyPress('p');
    Play(LeftPixel(AudioClip(0,3)) -2, 1000);
}

void TestBugs::testEndTrimAtOutTransitionInSavedDocumentEndCausesSnappingProblemVideo()
{
    StartTestSuite();
    TimelineZoomIn(6);
    {
        StartTest("Preparation");
        TimelineTrimRight(VideoClip(0,0), -300);
        TimelineDeleteClip(VideoClip(0,1));
        MakeOutTransitionAfterClip preparationVideo(0);
        preparationVideo.dontUndo();
    }
    pts length = getSequence()->getLength();
    ASSERT_EQUALS(length, getSequence()->getLength());
    DirAndFile tempDir_fileName = mProjectFixture.saveAndReload();
    ConfigOverrule<bool> overruleSnapToCursor(Config::sPathTimelineSnapClips,true);
    ConfigOverrule<bool> overruleSnapToClips(Config::sPathTimelineSnapCursor,false);
    {
        StartTest("Trim video");
        ASSERT_EQUALS(length, getSequence()->getLength());

        pts position = VideoClip(0, 1)->getRightPts();
        TimelineBeginTrim(UnderTransitionRightEdge(VideoClip(0,1)), false);
        ASSERT_EQUALS(position, VideoClip(0, 1)->getRightPts()); // Clip length should not have been extended yet.
        TimelineEndTrim(false);
    }
}

void TestBugs::testEndTrimAtOutTransitionInSavedDocumentEndCausesSnappingProblemAudio()
{
    StartTestSuite();
    TimelineZoomIn(6);
    {
        StartTest("Preparation video");
        TimelineTrimRight(VideoClip(0,0), -300);
        TimelineDeleteClip(VideoClip(0,1));
        MakeOutTransitionAfterClip preparationAudio(0, true);
        preparationAudio.dontUndo();
    }
    pts length = getSequence()->getLength();
    ASSERT_EQUALS(length, getSequence()->getLength());
    DirAndFile tempDir_fileName = mProjectFixture.saveAndReload();
    ConfigOverrule<bool> overruleSnapToCursor(Config::sPathTimelineSnapClips,true);
    ConfigOverrule<bool> overruleSnapToClips(Config::sPathTimelineSnapCursor,false);
    {
        StartTest("Trim audio");
        ASSERT_EQUALS(length, getSequence()->getLength());

        pts position = AudioClip(0, 1)->getRightPts();
        TimelineBeginTrim(UnderTransitionRightEdge(AudioClip(0,1)), false);
        ASSERT_EQUALS(position, AudioClip(0, 1)->getRightPts()); // Clip length should not have been extended yet.
        TimelineEndTrim(false);
    }
}

void TestBugs::testTrimAndExtendVideoAndAudioClipsThatBothHaveOutTransitions()
{
    StartTestSuite();
    TimelineZoomIn(6);

    TimelineDeleteClips({ VideoClip(0, 0), VideoClip(0, 2) });
    TimelineTrimRight(VideoClip(0,1), -100);
    TimelineTrimLeft(VideoClip(0,1), 100);
    TimelineSelectClips({});
    MakeOutTransitionAfterClip preparationVideo(1);
    MakeOutTransitionAfterClip preparationAudio(1,true);
    MakeInTransitionAfterClip preparationVideoIn(0);
    MakeInTransitionAfterClip preparationAudioIn(0, true);

    ASSERT_VIDEOTRACK0(EmptyClip)(Transition)(VideoClip)(Transition)(EmptyClip);
    ASSERT_AUDIOTRACK0(EmptyClip)(Transition)(AudioClip)(Transition)(EmptyClip);

    pts length = VideoClip(0,2)->getLength();
    ASSERT_EQUALS(AudioClip(0, 2)->getLength(), length);

    TimelineTrimTransitionLeftClipEnd(VideoClip(0,3), 100, false);
    ASSERT_MORE_THAN(VideoClip(0, 2)->getLength(), length);
    ASSERT_EQUALS(AudioClip(0, 2)->getLength(), VideoClip(0, 2)->getLength());
    Undo();
    ASSERT_EQUALS(VideoClip(0, 2)->getLength(), length);
    ASSERT_EQUALS(AudioClip(0, 2)->getLength(), length);

    TimelineTrimTransitionRightClipBegin(VideoClip(0,1), -100, false);
    ASSERT_MORE_THAN(VideoClip(0, 2)->getLength(), length);
    ASSERT_EQUALS(AudioClip(0, 2)->getLength(), VideoClip(0, 2)->getLength());
    Undo();
    ASSERT_EQUALS(VideoClip(0, 2)->getLength(), length);
    ASSERT_EQUALS(AudioClip(0, 2)->getLength(), length);

    TimelineTrimTransitionLeftClipEnd(AudioClip(0,3), 100, false);
    ASSERT_MORE_THAN(VideoClip(0, 2)->getLength(), length);
    ASSERT_EQUALS(AudioClip(0, 2)->getLength(), VideoClip(0, 2)->getLength());
    Undo();
    ASSERT_EQUALS(VideoClip(0, 2)->getLength(), length);
    ASSERT_EQUALS(AudioClip(0, 2)->getLength(), length);

    TimelineTrimTransitionRightClipBegin(AudioClip(0,1), -100, false);
    ASSERT_MORE_THAN(VideoClip(0, 2)->getLength(), length);
    ASSERT_EQUALS(AudioClip(0, 2)->getLength(), VideoClip(0, 2)->getLength());
    Undo();
    ASSERT_EQUALS(VideoClip(0, 2)->getLength(), length);
    ASSERT_EQUALS(AudioClip(0, 2)->getLength(), length);

}

void TestBugs::testCrashWhenEnlargingUnlinkedAudioClipBeyondFileLength()
{
    StartTestSuite();
    TimelinePositionCursor(5); // Not on clip
    Unlink(VideoClip(0,2));
    TimelineDeleteClips({ VideoClip(0, 2), AudioClip(0, 3) });;
    DirAndFile tempDir_fileName = mProjectFixture.saveAndReload();
    TimelineTrimRight(AudioClip(0,2),200); // Should not result in change
    ASSERT_EQUALS(AudioClip(0, 2)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,2));
}

void TestBugs::testCrashAfterSelectingAnEmptyClip()
{
    StartTestSuite();
    TimelineDeleteClip(VideoClip(0,1));
    ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip);
    TimelineLeftClick(Center(VideoClip(0,1)));
    ASSERT(DetailsView(nullptr));
    ASSERT_SELECTION_SIZE(0);
    TimelineLeftClick(Center(VideoClip(0,0)));
    ASSERT(DetailsView(VideoClip(0,0)));
    ASSERT_SELECTION_SIZE(1);
    TimelineLeftClick(Center(VideoClip(0,1)));
    ASSERT(DetailsView(VideoClip(0,0)));
    ASSERT_SELECTION_SIZE(1);
    TimelineKeyDown(WXK_CONTROL);
    TimelineLeftClick(Center(VideoClip(0,1)));
    TimelineKeyUp(WXK_CONTROL);
    ASSERT(DetailsView(VideoClip(0,0)));
    ASSERT_SELECTION_SIZE(1);
}

void TestBugs::testCrashWhenMovingCursorOutsideTimelineWhenShiftBeginTrimming()
{
    StartTestSuite();
    TimelineZoomIn(8);
    TimelineTrimLeft(VideoClip(0, 0), 20);
    TimelineBeginTrim(LeftCenter(VideoClip(0, 0)), true);
    pts l{ VideoClip(0,0)->getLength() };
    TimelineLeave(wxPoint(-1, VCenter(VideoClip(0, 0))));
    ASSERT_EQUALS(VideoClip(0, 0)->getLength(), l);
    TimelineEndTrim();
}

} // namespace
