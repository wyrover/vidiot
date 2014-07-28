// Copyright 2013,2014 Eric Raijmakers.
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
    TimelineLeftClick(VTopQuarterHCenter(VideoClip(0,2)));
    DeselectAllClips();
}

void TestBugs::testDetailsNotShownAfterMovingTimelineCursor()
{
    StartTestSuite();
    TimelineLeftClick(Center(VideoClip(0,3)));
    PositionCursor(HCenter(VideoClip(0,3)));
    TimelineLeftClick(Center(VideoClip(0,3)));
    ASSERT_DETAILSCLIP(VideoClip(0,3));
}

void TestBugs::testLinkingErrorWhenDroppingOverBeginOfLinkedClip()
{
    StartTestSuite();
    TriggerMenu(ID_ADDVIDEOTRACK);
    TrimLeft(VideoClip(0,4),40,false);
    Drag(From(Center(VideoClip(0,6))).To(wxPoint(RightPixel(VideoClip(0,4)),VCenter(getSequence()->getVideoTrack(1)))));
    ASSERT_EQUALS(VideoClip(0,5)->getLink(),AudioClip(0,6));
    TrimLeft(VideoClip(0,5),10,false); // This caused an assert, because there was a problem with this clip (video(0,5)) link.
}

void TestBugs::testErrorInGetNextHandlingForEmptyClips()
{
    StartTestSuite();
    TriggerMenu(ID_ADDVIDEOTRACK);
    TriggerMenu(ID_ADDVIDEOTRACK);

    DragToTrack(1,VideoClip(0,5),model::IClipPtr());
    Drag(From(Center(VideoClip(1,1))).To(wxPoint(HCenter(VideoClip(0,4)),VCenter(VideoClip(1,1)))));

    DragToTrack(2,VideoClip(0,6),model::IClipPtr());
    Drag(From(Center(VideoClip(2,1))).AlignLeft(LeftPixel(VideoClip(1,1))));

    TimelineLeftClick(Center(VideoClip(1,1)));
    MouseClickTopLeft(DetailsClipView()->getScalingSlider()); // Give focus
    KeyboardKeyPressN(6,WXK_PAGEUP);
    MouseClickTopLeft(DetailsClipView()->getPositionXSlider()); // Give focus
    KeyboardKeyPressN(4,WXK_PAGEDOWN);
    MouseClickTopLeft(DetailsClipView()->getPositionYSlider()); // Give focus
    KeyboardKeyPressN(4,WXK_PAGEDOWN);

    TimelineLeftClick(Center(VideoClip(2,1)));
    MouseClickTopLeft(DetailsClipView()->getScalingSlider()); // Give focus
    KeyboardKeyPressN(6,WXK_PAGEUP);
    MouseClickTopLeft(DetailsClipView()->getPositionXSlider()); // Give focus
    KeyboardKeyPressN(4,WXK_PAGEUP);
    MouseClickTopLeft(DetailsClipView()->getPositionYSlider()); // Give focus
    KeyboardKeyPressN(4,WXK_PAGEUP);

    PositionCursor(LeftPixel(VideoClip(2,1)) - 5);

    Play(LeftPixel(VideoClip(2,1)) - 3,2500); // Start before the clip, in the empty area. Due to a bug in 'getNext' handling for empty clips the clips after the empty area were not shown, or too late.

    Undo(9);
}

void TestBugs::testDraggingWithoutSelection()
{
    StartTestSuite();

    TimelineLeftClick(Center(VideoClip(0,1))); // Select the clip
    TimelineKeyDown(wxMOD_CONTROL);
    TimelineLeftDown(); // Deselects the clip already
    TimelineMove(Center(VideoClip(0,3))); // Starts the drag without anything being selected: ASSERT at the time of the bug. Now the drag should be simply omitted.
    ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();
    TimelineKeyUp(wxMOD_CONTROL);
    TimelineLeftUp();
}

void TestBugs::testBugsWithLongTimeline()
{
    StartTestSuite();
    model::SequencePtr sequence = getSequence();
    Config::setShowDebugInfo(true);
    TriggerMenu(ID_CLOSESEQUENCE);

    extendSequenceWithRepeatedClips(sequence, mProjectFixture.InputFiles, 30);

    openTimelineForSequence(sequence);
    Zoom level(4);
    TimelineKeyPress(WXK_END); // Move scrollbars to end

    RunInMainAndWait([]
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
        PositionCursor(HCenter(VideoClip(0,-2)));
        // NOTE: Don't use waitForIdle() when the video is playing!!!
        //       When the video is playing, the system does not become Idle (playback events).
        TimelineKeyPress(' ');
        SetWaitAfterEachInputAction(false); // Avoid waitForIdle during playback
        pause(1000);
        TimelineKeyDown(wxMOD_SHIFT);
        pause(1000);
        TimelineKeyUp(wxMOD_SHIFT);
        pause(1000);
        TimelineKeyPress(' ');
        waitForIdle();
        SetWaitAfterEachInputAction(true);
        TimelineLeftClick(Center(VideoClip(0,-2)));
        TimelineKeyPress(WXK_DELETE);
        ASSERT(VideoClip(0,-2)->isA<model::EmptyClip>());
    }
    std::pair<RandomTempDirPtr, wxFileName> tempDir_fileName;
    {
        StartTest("Bug: StackOverflow when saving");
        tempDir_fileName = SaveProjectAndClose();
    }
    {
        StartTest("Bug: StackOverflow when loading");
        RunInMainAndWait([tempDir_fileName]()
        {
            gui::Window::get().GetDocumentManager()->CreateDocument(tempDir_fileName.second.GetFullPath(),wxDOC_SILENT);
        });
    }
    TriggerMenu(wxID_CLOSE);
    Config::setShowDebugInfo(false);
}

void TestBugs::testPlaybackEmptyClip()
{
    StartTestSuite();

    TimelineLeftClick(Center(VideoClip(0,3)));
    TimelineKeyPress(WXK_DELETE);
    PositionCursor(HCenter(VideoClip(0,3)));
    model::VideoFramePtr frame = boost::dynamic_pointer_cast<model::EmptyClip>(VideoClip(0,3))->getNextVideo(model::VideoCompositionParameters().setBoundingBox(wxSize(100,100)));
    ASSERT_NONZERO(frame);
    model::AudioChunkPtr chunk = boost::dynamic_pointer_cast<model::EmptyClip>(AudioClip(0,3))->getNextAudio(model::AudioCompositionParameters());
    ASSERT_NONZERO(chunk);
    // Note: do not pause() or press space at this point. The getNexts above 'mess up' the administration (audioclip is already at end, but the cursor position is not)
    Play(RightPixel(VideoClip(0,3)) - 3,2000);
}

void TestBugs::testTrimmingClipAdjacentToZeroLengthClipUsedForTransition()
{
    StartTestSuite();
    Zoom level(6);
    {
        StartTest("Clip to the right of the trim has length 0.");
        OpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('o'); // fade &out
        ShiftTrim(LeftCenter(VideoClip(0,1)), RightCenter(VideoClip(0,1))  + wxPoint(10,0)); // Create a clip with length 0
        ASSERT_ZERO(VideoClip(0,1)->getLength());
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition);
        ShiftTrim(RightCenter(VideoClip(0,0)), Center(VideoClip(0,0))); // Caused crash in trim handling due to 'adjacent clip' having length 0
        Undo(3);
    }
    {
        StartTest("Clip to the left of the trim has length 0.");
        OpenPopupMenuAt(Center(VideoClip(0,0)));
        TimelineKeyPress('i'); // fade &in
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
    TimelineKeyPress('t');
    ASSERT_EQUALS(VideoClip(0,1)->getLeftPts(), AudioClip(0,1)->getLeftPts());
    ASSERT_EQUALS(VideoTrack(0)->getLength(),AudioTrack(0)->getLength());
    ASSERT_EQUALS(VideoClip(0,4)->getRightPts(), AudioClip(0,4)->getRightPts());
    Undo();
}

void TestBugs::testCrashWhenDroppingPartiallyOverATransition()
{
    StartTestSuite();
    Zoom level(4);
    {
        MakeInOutTransitionAfterClip preparation(1,true);
        {
            StartTest("AudioTransition: Drop a clip with its left edge exactly on the middle of a transition");
            Drag(From(Center(AudioClip(0,4))).AlignLeft(HCenter(AudioClip(0,2))));
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(EmptyClip);
            ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(EmptyClip);
            ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
            ASSERT_EQUALS(AudioClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,3));
            Undo();
        }
        {
            StartTest("AudioTransition: Drop a clip with its left edge inside the right half of a transition");
            Drag(From(Center(AudioClip(0,4))).AlignLeft(HCenter(AudioClip(0,2)) + 5));
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(EmptyClip);
            ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip)(EmptyClip);
            ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
            ASSERT_EQUALS(AudioClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,3));
            Undo();
        }
        {
            StartTest("AudioTransition: Drop a clip with its right edge inside the left half of a transition");
            Drag(From(Center(AudioClip(0,4))).AlignRight(HCenter(AudioClip(0,2)) - 5));
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
            Drag(From(Center(VideoClip(0,4))).AlignLeft(HCenter(VideoClip(0,2))));
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(EmptyClip);
            ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(EmptyClip);
            ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
            ASSERT_EQUALS(AudioClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,3));
            Undo();
        }
        {
            StartTest("VideoTransition: Drop a clip with its left edge inside the right half of a transition");
            Drag(From(Center(VideoClip(0,4))).AlignLeft(HCenter(VideoClip(0,2)) + 5));
            ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(EmptyClip);
            ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip)(EmptyClip);
            ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
            ASSERT_EQUALS(AudioClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,3));
            Undo();
        }
        {
            StartTest("VideoTransition: Drop a clip with its right edge inside the left half of a transition");
            Drag(From(Center(VideoClip(0,4))).AlignRight(HCenter(VideoClip(0,2)) - 5));
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
    Zoom level(2);
    TriggerMenu(ID_ADDVIDEOTRACK);
    TriggerMenu(ID_ADDAUDIOTRACK);
    DragToTrack(1,VideoClip(0,2),AudioClip(0,2));
    ASSERT_VIDEOTRACK1(     EmptyClip      )(VideoClip);
    ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(EmptyClip);
    ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(EmptyClip);
    ASSERT_AUDIOTRACK1(     EmptyClip      )(AudioClip);
    {
        StartTest("EndTrim allowed even with adjacent touching clip in other track");
        TrimRight(VideoClip(0,1), -20);
        ASSERT_LESS_THAN(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        Undo();
    }
    {
        StartTest("EndTrim allowed even with adjacent touching clip in other track");
        TrimLeft(VideoClip(0,1), 20);
        ASSERT_LESS_THAN(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        Undo();
    }
}

void TestBugs::testAddNonexistentFileViaRedo()
{
    StartTestSuite();

    // Create temp dir with temp file
    RandomTempDirPtr tempDir = boost::make_shared<RandomTempDir>();
    model::IPaths inputfiles = getListOfInputFiles();
    wxString aviFileName = inputfiles.front()->getPath().GetLongPath();
    wxFileName filepath(tempDir->getFileName().GetLongPath(),"file","avi");
    bool copyok = wxCopyFile( aviFileName, filepath.GetLongPath(), false );
    ASSERT(copyok);

    // Add folder to project view
    wxString sFolder1( "Folder1" );
    model::FolderPtr folder1 = addFolder( sFolder1 );

    // Add file to folder
    model::Files files1 = addFiles( boost::assign::list_of(filepath), folder1 );
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
    model::SequencePtr sequence1 = createSequence( folder1 );

}

void TestBugs::testAddNonexistentFileViaUndo()
{
    StartTestSuite();

    // Create temp dir with temp file
    RandomTempDirPtr tempDir = boost::make_shared<RandomTempDir>();
    model::IPaths inputfiles = getListOfInputFiles();
    wxString aviFileName = inputfiles.front()->getPath().GetLongPath();
    wxFileName filepath(tempDir->getFileName().GetLongPath(),"file","avi");
    bool copyok = wxCopyFile( aviFileName, filepath.GetLongPath(), false );
    ASSERT(copyok);

    // Add folder to project view
    wxString sFolder1( "Folder1" );
    model::FolderPtr folder1 = addFolder( sFolder1 );

    // Add file to folder
    model::Files files1 = addFiles( boost::assign::list_of(filepath), folder1 );
    ASSERT_WATCHED_PATHS_COUNT(2);

    // Remove the file via 'Delete'
    remove(files1.front());
    ASSERT_WATCHED_PATHS_COUNT(1);

    // Remove the file from disk
    tempDir.reset();

    // Add the file again (via 'Undo')
    gui::Dialog::get().setConfirmation(); // Trigger ok when the application informs that the file was deleted.
    Undo();
    ASSERT_WATCHED_PATHS_COUNT(1);

    // Now make the sequence (is empty)
    model::SequencePtr sequence1 = createSequence( folder1 );

}

void TestBugs::testPlaybackWithMultipleAudioTracks()
{
    StartTestSuite();

    TriggerMenu(ID_ADDAUDIOTRACK);
    DragToTrack(1,model::IClipPtr(),AudioClip(0,2));
    Play(HCenter(AudioClip(0,0)), 1000);
}


} // namespace
