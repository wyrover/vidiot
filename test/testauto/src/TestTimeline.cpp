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

#include "TestTimeline.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestTimeline::setUp()
{
    mProjectFixture.init();
}

void TestTimeline::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestTimeline::testSelection()
{
    StartTestSuite();
    TimelineZoomIn(4);
    const model::IClips& clips = getSequence()->getVideoTrack(0)->getClips();
    {
        StartTest("Start application, make sequence, shift click clip five. Only clip five selected!");
        TimelineSelectClips({});
        ASSERT_SELECTION_SIZE(0);
        TimelineKeyDown(WXK_SHIFT);
        TimelineLeftClick(Center(VideoClip(0,4)));
        TimelineKeyUp(WXK_SHIFT);
        ASSERT_SELECTION_SIZE(1);
    }
    {
        StartTest("CTRL clicking all clips one by one");
        TimelineSelectClips({});
        TimelineKeyDown(WXK_CONTROL);
        for (model::IClipPtr clip : clips)
        {
            TimelineMove(Center(clip));
            TimelineLeftDown();
            ASSERT(clip->getSelected()); // Clip selected on mouse down event
            TimelineLeftUp();
        }
        TimelineKeyUp(WXK_CONTROL);
        ASSERT_SELECTION_SIZE(mProjectFixture.InputFiles.size());
    }
    {
        StartTest("SHIFT clicking the entire list");
        TimelineSelectClips({});
        ASSERT_SELECTION_SIZE(0);
        TimelineKeyDown(WXK_SHIFT);
        TimelineLeftClick(Center(clips.front()));
        TimelineLeftClick(Center(clips.back()));
        TimelineKeyUp(WXK_SHIFT);
        ASSERT_SELECTION_SIZE(mProjectFixture.InputFiles.size());
    }
    {
        StartTest("SHIFT clicking only the partial list");
        TimelineSelectClips({});
        ASSERT_SELECTION_SIZE(0);
        TimelineLeftClick(Center(VideoClip(0,2)));
        TimelineKeyDown(WXK_SHIFT);
        TimelineLeftClick(Center(VideoClip(0,4)));
        TimelineKeyUp(WXK_SHIFT);
        ASSERT_SELECTION_SIZE(3);
    }
    {
        StartTest("(de)selecting one clip with CTRL click");
        TimelineKeyDown(WXK_CONTROL);
        TimelineLeftClick(Center(VideoClip(0,3)));
        TimelineKeyUp(WXK_CONTROL);
        ASSERT_SELECTION_SIZE(2);
        TimelineKeyDown(WXK_CONTROL);
        TimelineLeftClick(Center(VideoClip(0,3)));
        TimelineKeyUp(WXK_CONTROL);
        ASSERT_SELECTION_SIZE(3);
    }
    {
        StartTest("Select the transition between two clips when shift selecting.");
        TimelineSelectClips({});
        MakeInOutTransitionAfterClip preparation(1);
        ASSERT(VideoClip(0,2)->isA<model::Transition>());
        TimelineLeftClick(Center(VideoClip(0,1)));
        TimelineKeyDown(WXK_SHIFT);
        TimelineLeftClick(Center(VideoClip(0,3)));
        TimelineKeyUp(WXK_SHIFT);
        ASSERT(VideoClip(0,2)->isA<model::Transition>() && VideoClip(0,2)->getSelected());
    }
    {
        StartTest("Select an in-out-transition.");
        MakeInOutTransitionAfterClip preparation(1);
        TimelineSelectClips({});
        TimelineLeftClick(VTopQuarterHCenter(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        TimelineSelectClips({});
        TimelineLeftClick(VTopQuarterLeft(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        TimelineSelectClips({});
        TimelineLeftClick(VTopQuarterRight(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        TimelineSelectClips({});
    }
    {
        StartTest("Select an out-only-transition.");
        MakeOutTransitionAfterClip preparation(1);
        TimelineSelectClips({});
        TimelineLeftClick(VTopQuarterHCenter(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        TimelineSelectClips({});
        TimelineLeftClick(VTopQuarterLeft(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        TimelineSelectClips({});
        TimelineLeftClick(VTopQuarterRight(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        TimelineSelectClips({});
    }
    {
        StartTest("Select an in-only-transition.");
        MakeInTransitionAfterClip preparation(1);
        TimelineSelectClips({});
        TimelineLeftClick(VTopQuarterHCenter(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        TimelineSelectClips({});
        TimelineLeftClick(VTopQuarterLeft(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        TimelineSelectClips({});
        TimelineLeftClick(VTopQuarterRight(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        TimelineSelectClips({});
    }
}

void TestTimeline::testSelectionMultipleTracks()
{
    StartTestSuite();
    WindowTriggerMenu(ID_ADDVIDEOTRACK);
    WindowTriggerMenu(ID_ADDAUDIOTRACK);
    TimelineDragToTrack(1, VideoClip(0, 3), AudioClip(0, 3));
    TimelineDrag(From(Center(VideoClip(1, 1))).AlignLeft(HCenter(VideoClip(0, 1))));
    TimelineDrag(From(Center(AudioClip(1, 1))).AlignLeft(HCenter(VideoClip(0, 1))));
    ASSERT_VIDEOTRACK1(EmptyClip)     (VideoClip);
    ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(VideoClip)(VideoClip);
    ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip)(AudioClip);
    ASSERT_AUDIOTRACK1(EmptyClip)     (AudioClip);
    ASSERT_MORE_THAN(VideoClip(1, 1)->getLeftPts(), VideoClip(0, 1)->getLeftPts());
    ASSERT_LESS_THAN(VideoClip(1, 1)->getLeftPts(), VideoClip(0, 1)->getRightPts());
    ASSERT_MORE_THAN(AudioClip(1, 1)->getLeftPts(), AudioClip(0, 1)->getLeftPts());
    ASSERT_LESS_THAN(AudioClip(1, 1)->getLeftPts(), AudioClip(0, 1)->getRightPts());
    TimelineDragToTrack(1, VideoClip(0, 5), AudioClip(0, 5));
    {
        StartTest("Select all");
        TimelineSelectClips({});
        TimelineKeyDown(WXK_ALT);
        TimelineLeftClick(Center(VideoClip(0,0)));
        TimelineKeyUp(WXK_ALT);
        ASSERT_SELECTION_SIZE(7);
    }
    {
        StartTest("Select before clip - in track 2 - begin");
        TimelineSelectClips({});
        TimelineKeyDown(WXK_ALT);
        TimelineLeftClick(Center(VideoClip(0,1)));
        TimelineKeyUp(WXK_ALT);
        ASSERT_SELECTION_SIZE(6);
    }
    {
        StartTest("Select clip in track 2");
        TimelineSelectClips({});
        TimelineKeyDown(WXK_ALT);
        TimelineLeftClick(Center(VideoClip(1,1)));
        TimelineKeyUp(WXK_ALT);
        ASSERT_SELECTION_SIZE(5);
    }
    {
        StartTest("Select after clip - in track 2 - begin");
        TimelineSelectClips({});
        TimelineKeyDown(WXK_ALT);
        TimelineLeftClick(Center(VideoClip(0,2)));
        TimelineKeyUp(WXK_ALT);
        ASSERT_SELECTION_SIZE(4);
    }
}


void TestTimeline::testDeletion()
{
    StartTestSuite();
    {
        StartTest("When deleting without shift, a clip is replaced with emptyness.");
        pts len = VideoTrack(0)->getLength();
        int num = NumberOfVideoClipsInTrack(0);
        TimelineSelectClips({});
        TimelineLeftClick(Center(VideoClip(0,1)));
        TimelineKeyDown(WXK_CONTROL);
        TimelineLeftClick(Center(VideoClip(0,3)));
        ASSERT_SELECTION_SIZE(2);
        TimelineKeyUp(WXK_CONTROL);
        TimelineKeyPress(WXK_DELETE);
        ASSERT_SELECTION_SIZE(0);
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(EmptyClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip)(EmptyClip)(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoTrack(0)->getLength(),len);
        ASSERT_EQUALS(NumberOfVideoClipsInTrack(0),num);
        TimelineLeftClick(Center(VideoClip(0,5)));
        ASSERT_SELECTION_SIZE(1);
        TimelineKeyPress(WXK_DELETE);
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(EmptyClip)(VideoClip)(EmptyClip);
        ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip)(EmptyClip)(AudioClip)(EmptyClip);
        ASSERT_EQUALS(VideoTrack(0)->getLength(),len);
        ASSERT_EQUALS(NumberOfVideoClipsInTrack(0),num);
        ASSERT_SELECTION_SIZE(0);
        Undo();
        ASSERT_SELECTION_SIZE(1);
        Undo();
        ASSERT_SELECTION_SIZE(2);
    }
    {
        StartTest("When deleting with shift, a clip is replaced with emptyness and then the emptyness is removed.");
        pts len = VideoTrack(0)->getLength();
        int num = NumberOfVideoClipsInTrack(0);
        TimelineSelectClips({});
        TimelineLeftClick(Center(VideoClip(0,1)));
        TimelineKeyDown(WXK_CONTROL);
        TimelineLeftClick(Center(VideoClip(0,3)));
        TimelineKeyUp(WXK_CONTROL);
        TimelineKeyDown(WXK_SHIFT);
        ASSERT_SELECTION_SIZE(2);
        TimelineKeyPress(WXK_DELETE);
        TimelineKeyUp(WXK_SHIFT);
        ASSERT_SELECTION_SIZE(0);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip);
        ASSERT_LESS_THAN(VideoTrack(0)->getLength(),len);
        ASSERT_EQUALS(NumberOfVideoClipsInTrack(0),num-2);
        TimelineLeftClick(Center(VideoClip(0,2)));
        ASSERT_SELECTION_SIZE(1);
        TimelineKeyDown(WXK_SHIFT);
        TimelineKeyPress(WXK_DELETE);
        TimelineKeyUp(WXK_SHIFT);
        ASSERT_SELECTION_SIZE(0);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip);
        ASSERT_EQUALS(NumberOfVideoClipsInTrack(0),num-3);
        Undo();
        ASSERT_SELECTION_SIZE(1);
        Undo();
        ASSERT_SELECTION_SIZE(2);
        TimelineSelectClips({});
    }
    {
        StartTest("When deleting a clip, an in-only transition must be deleted also.");
        MakeInTransitionAfterClip preparation(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        TimelineSelectClips({});
        TimelineLeftClick(Center(VideoClip(0,3)));
        TimelineKeyPress(WXK_DELETE);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(     EmptyClip       )(VideoClip);
        ASSERT_SELECTION_SIZE(0);
        Undo(1);
    }
    {
        StartTest("When deleting a clip, a out-only transition must be deleted also.");
        MakeOutTransitionAfterClip preparation(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        TimelineSelectClips({});
        TimelineLeftClick(Center(VideoClip(0,1)));
        TimelineKeyPress(WXK_DELETE);
        ASSERT_VIDEOTRACK0(VideoClip)(     EmptyClip       )(VideoClip);
        ASSERT_SELECTION_SIZE(0);
        Undo(1);
    }
};

void TestTimeline::testDeletionWithUnlinkedClips()
{
    StartTestSuite();
    {
        StartTest("AudioClip: When deleting with shift, all tracks must be shifted, even if no clips are selected in them.");
        TimelineSelectClips({});
        Unlink(VideoClip(0,2));
        TimelineLeftClick(Center(VideoClip(0,2)));
        TimelineKeyPress(WXK_DELETE);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(EmptyClip)(VideoClip);
        TimelineLeftClick(Center(AudioClip(0,2)));
        TimelineKeyDown(WXK_SHIFT);
        TimelineKeyPress(WXK_DELETE);
        TimelineKeyUp(WXK_SHIFT);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
        ASSERT_EQUALS(AudioClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,1));
        ASSERT_EQUALS(AudioClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,3));
        Undo(3);
    }
    {
        StartTest("VideoClip: When deleting with shift, all tracks must be shifted, even if no clips are selected in them.");
        TimelineSelectClips({});
        Unlink(AudioClip(0,2));
        TimelineLeftClick(Center(AudioClip(0,2)));
        TimelineKeyPress(WXK_DELETE);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(EmptyClip)(AudioClip);
        TimelineLeftClick(Center(VideoClip(0,2)));
        TimelineKeyDown(WXK_SHIFT);
        TimelineKeyPress(WXK_DELETE);
        TimelineKeyUp(WXK_SHIFT);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
        ASSERT_EQUALS(AudioClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,1));
        ASSERT_EQUALS(AudioClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,3));
        Undo(3);
    }
    {
        StartTest("AudioClip: When deleting with shift, and another track has a non selected clip in the to-be-shifted region, do not shift.");
        TimelineSelectClips({});
        Unlink(VideoClip(0,2));
        TimelineLeftClick(Center(VideoClip(0,2)));
        TimelineTrimLeft(VideoClip(0,2), 20, false);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(EmptyClip)(VideoClip)(VideoClip);
        TimelineLeftClick(Center(AudioClip(0,2)));
        TimelineKeyDown(WXK_SHIFT);
        TimelineKeyPress(WXK_DELETE);
        TimelineKeyUp(WXK_SHIFT);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(EmptyClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(EmptyClip)(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_DIFFERS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
        ASSERT_EQUALS(AudioClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,1));
        ASSERT_EQUALS(AudioClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,3));
        Undo(3);
    }
    {
        StartTest("VideoClip: When deleting with shift, and another track has a non selected clip in the to-be-shifted region, do not shift.");
        TimelineSelectClips({});
        Unlink(AudioClip(0,2));
        TimelineLeftClick(Center(AudioClip(0,2)));
        TimelineTrimLeft(AudioClip(0,2), 20, false);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(EmptyClip)(AudioClip)(AudioClip);
        TimelineLeftClick(Center(VideoClip(0,2)));
        TimelineKeyDown(WXK_SHIFT);
        TimelineKeyPress(WXK_DELETE);
        TimelineKeyUp(WXK_SHIFT);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(EmptyClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(EmptyClip)(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
        ASSERT_EQUALS(AudioClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,1));
        ASSERT_DIFFERS(AudioClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,3));
        ASSERT_EQUALS(AudioClip(0,4)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,3));
        Undo(3);
    }
}

void TestTimeline::testUndo()
{
    StartTestSuite();
    TimelineZoomIn(2);
    ConfigFixture.SnapToClips(true);
    pts length = VideoClip(0,3)->getLength();
    TimelineDrag(From(Center(VideoClip(0,3))).To(wxPoint(2,Center(VideoClip(0,3)).y)));
    ASSERT_EQUALS(VideoClip(0,0)->getLength(),length);
    Undo();
    ASSERT_EQUALS(VideoClip(0,3)->getLength(),length);

    TimelineKeyPress('=');  // Zoom in
    MakeInOutTransitionAfterClip preparation(1);
    ASSERT_EQUALS(VideoTrack(0)->getLength(),AudioTrack(0)->getLength());

    // Move clip 2: the transition must be removed
    TimelineSelectClips({});
    TimelineDrag(From(Center(VideoClip(0,1))).To(Center(VideoClip(0,4))));
    ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
    ASSERT(!VideoClip(0,2)->isA<model::Transition>());
    Undo();
    ASSERT(!VideoClip(0,1)->isA<model::EmptyClip>());
    ASSERT(VideoClip(0,2)->isA<model::Transition>());

    // Move clip 3: the transition must be removed and the fourth clip becomes the third one (clip+transition removed)
    model::IClipPtr afterclip = VideoClip(0,4);
    TimelineSelectClips({});
    TimelineDrag(From(Center(VideoClip(0,3))).To(Center(VideoClip(0,5))));
    ASSERT_EQUALS(afterclip,VideoClip(0,3));
    ASSERT(VideoClip(0,2)->isA<model::EmptyClip>());
    ASSERT(!VideoClip(0,2)->isA<model::Transition>());
    ASSERT_HISTORY_END
        (cmd::ProjectViewCreateAutoFolder)
        (cmd::ProjectViewCreateSequence)
        (gui::timeline::cmd::TrimClip)
        (gui::timeline::cmd::TrimClip)
        (gui::timeline::cmd::CreateTransition)
        (gui::timeline::cmd::ExecuteDrop);
    Undo(5); 
    ASSERT_HISTORY_END(cmd::ProjectViewCreateAutoFolder);
    Redo(5);
    ASSERT_HISTORY_END
        (cmd::ProjectViewCreateAutoFolder)
        (cmd::ProjectViewCreateSequence)
        (gui::timeline::cmd::TrimClip)
        (gui::timeline::cmd::TrimClip)
        (gui::timeline::cmd::CreateTransition)
        (gui::timeline::cmd::ExecuteDrop);
    Undo();
}

void TestTimeline::testAbortDrag()
{
    StartTestSuite();
    for (int zoom = 0; zoom < 4; zoom++)
    {
        ASSERT_HISTORY_END(cmd::ProjectViewCreateSequence);

        TimelineSelectClips({});
        TimelineDrag(From(Center(VideoClip(0,5))).To(Center(VideoClip(0,4))).DontReleaseMouse());
        TimelineKeyDown(WXK_SHIFT);
        TimelineMove(Center(VideoClip(0,3)));
        TimelineKeyPress(WXK_ESCAPE); // Abort the drop
        TimelineLeftUp();
        TimelineKeyUp(WXK_SHIFT);

        ASSERT_MORE_THAN_EQUALS(getTimeline().getZoom().pixelsToPts(LeftCenter(VideoClip(0,1)).x),VideoClip(0,1)->getLeftPts());

        for (unsigned int clipnum = 0; clipnum < mProjectFixture.InputFiles.size(); clipnum++)
        {
            ASSERT_EQUALS(VideoClip(0,clipnum)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,clipnum));
            ASSERT_EQUALS(AudioClip(0,clipnum)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,clipnum));
        }

        // NOT: Undo(); -- Drag was aborted so nothing was changed
        TimelineKeyPress('=');  // Zoom in and test again
    }
}

void TestTimeline::testDividers()
{
    StartTestSuite();
    const pixel changeY = 20; // Number of pixels to move the divider
    const pixel fixedX = 100; // Fixed x position on timeline
    const pixel moveToMiddleOfDivider = 2; // Click somewhere in the middle of a divider

    auto DragDivider = [fixedX,moveToMiddleOfDivider](pixel from, pixel to)
    {
        wxPoint original(fixedX, from + moveToMiddleOfDivider);
        wxPoint adjusted(fixedX, to + moveToMiddleOfDivider);
        TimelineMove(original);
        TimelineLeftDown();
        TimelineMove(adjusted);
        TimelineLeftUp();
    };

    {
        StartTest("Move the audio/video divider down and up again.");
        const pixel originalDividerPosition = getSequence()->getDividerPosition();
        const pixel adjustedDividerPosition = originalDividerPosition + changeY;
        DragDivider(originalDividerPosition,adjustedDividerPosition);
        TimelineMove(wxPoint(fixedX, 10)); // Was a bug once: the mouse release did not 'release' the move operation, and thus this move back up caused the divider back to its original position.
        ASSERT_EQUALS(getSequence()->getDividerPosition(), adjustedDividerPosition);
        DragDivider(adjustedDividerPosition,originalDividerPosition);
        ASSERT_EQUALS(getSequence()->getDividerPosition(), originalDividerPosition);
    }
    {
        StartTest("Move audio track divider up and down again.");
        const pixel originalHeight = AudioTrack(0)->getHeight();
        const pixel originalDividerPosition = getTimeline().getViewMap().getView(AudioTrack(0))->getY() + AudioTrack(0)->getHeight();
        const pixel adjustedDividerPosition = originalDividerPosition - changeY;
        DragDivider(originalDividerPosition, adjustedDividerPosition);
        TimelineMove(wxPoint(fixedX, 10)); // Was a bug once: the mouse release did not 'release' the move operation, and thus this move back up caused the divider back to its original position.
        ASSERT_EQUALS(AudioTrack(0)->getHeight(), originalHeight - changeY);
        DragDivider(adjustedDividerPosition, originalDividerPosition);
        ASSERT_EQUALS(AudioTrack(0)->getHeight(), originalHeight);
    }
    {
        StartTest("Move video track divider down and up again.");
        const pixel originalHeight = VideoTrack(0)->getHeight();
        const pixel originalDividerPosition = getTimeline().getViewMap().getView(VideoTrack(0))->getY() - gui::timeline::DividerView::TrackDividerHeight;
        const pixel adjustedDividerPosition = originalDividerPosition + changeY;
        DragDivider(originalDividerPosition, adjustedDividerPosition);
        TimelineMove(wxPoint(fixedX, 10)); // Was a bug once: the mouse release did not 'release' the move operation, and thus this move back up caused the divider back to its original position.
        ASSERT_EQUALS(VideoTrack(0)->getHeight(), originalHeight - changeY);
        DragDivider(adjustedDividerPosition, originalDividerPosition);
        ASSERT_EQUALS(VideoTrack(0)->getHeight(), originalHeight);
    }
}

void TestTimeline::testTrimming()
{
    StartTestSuite();
    TimelineZoomIn(2);
    TimelineDeleteClips({ VideoClip(0, 3), VideoClip(0, 1) });
    ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(EmptyClip)(VideoClip);
    ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip)(EmptyClip)(AudioClip);
    {
        StartTest("Trim: Without Shift: Reduce clip size left.");
        TimelineTrimLeft(VideoClip(0,2),20,false);
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(EmptyClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip)(EmptyClip)(AudioClip);
        ASSERT_MORE_THAN(VideoClip(0,1)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_LESS_THAN(VideoClip(0,2)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,2));
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,3));
        ASSERT_EQUALS(VideoClip(0,4)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,4));
        ASSERT_EQUALS(VideoClip(0,4)->getLeftPts(),mProjectFixture.OriginalPtsOfVideoClip(0,4));
        ASSERT_MORE_THAN(AudioClip(0,1)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,1));
        ASSERT_LESS_THAN(AudioClip(0,2)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,2));
        ASSERT_EQUALS(AudioClip(0,3)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,3));
        ASSERT_EQUALS(AudioClip(0,4)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,4));
        ASSERT_EQUALS(AudioClip(0,4)->getLeftPts(),mProjectFixture.OriginalPtsOfAudioClip(0,4));
        StartTest("Trim: Without Shift: Enlarge clip size left.");
        TimelineTrimLeft(VideoClip(0,2),-20,false);
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(EmptyClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip)(EmptyClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,2));
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,3));
        ASSERT_EQUALS(VideoClip(0,4)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,4));
        ASSERT_EQUALS(VideoClip(0,4)->getLeftPts(),mProjectFixture.OriginalPtsOfVideoClip(0,4));
        ASSERT_EQUALS(AudioClip(0,1)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,1));
        ASSERT_EQUALS(AudioClip(0,2)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,2));
        ASSERT_EQUALS(AudioClip(0,3)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,3));
        ASSERT_EQUALS(AudioClip(0,4)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,4));
        ASSERT_EQUALS(AudioClip(0,4)->getLeftPts(),mProjectFixture.OriginalPtsOfAudioClip(0,4));
        Undo();
        Undo();
    }
    {
        StartTest("Trim: Without Shift: Reduce clip size right.");
        TimelineTrimRight(VideoClip(0,2),-20,false);
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(EmptyClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip)(EmptyClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_LESS_THAN(VideoClip(0,2)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,2));
        ASSERT_MORE_THAN(VideoClip(0,3)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,3));
        ASSERT_EQUALS(VideoClip(0,4)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,4));
        ASSERT_EQUALS(VideoClip(0,4)->getLeftPts(),mProjectFixture.OriginalPtsOfVideoClip(0,4));
        ASSERT_EQUALS(AudioClip(0,1)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,1));
        ASSERT_LESS_THAN(AudioClip(0,2)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,2));
        ASSERT_MORE_THAN(AudioClip(0,3)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,3));
        ASSERT_EQUALS(AudioClip(0,4)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,4));
        ASSERT_EQUALS(AudioClip(0,4)->getLeftPts(),mProjectFixture.OriginalPtsOfAudioClip(0,4));
        StartTest("Trim: Without Shift: Enlarge clip size right.");
        TimelineTrimRight(VideoClip(0,2),20,false);
        ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip)(EmptyClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip)(EmptyClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,2));
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,3));
        ASSERT_EQUALS(VideoClip(0,4)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,4));
        ASSERT_EQUALS(VideoClip(0,4)->getLeftPts(),mProjectFixture.OriginalPtsOfVideoClip(0,4));
        ASSERT_EQUALS(AudioClip(0,1)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,1));
        ASSERT_EQUALS(AudioClip(0,2)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,2));
        ASSERT_EQUALS(AudioClip(0,3)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,3));
        ASSERT_EQUALS(AudioClip(0,4)->getLength(),mProjectFixture.OriginalLengthOfAudioClip(0,4));
        ASSERT_EQUALS(AudioClip(0,4)->getLeftPts(),mProjectFixture.OriginalPtsOfAudioClip(0,4));
        Undo();
        Undo();
    }
    Undo();
    {
        StartTest("Trim: Left: During the trim, the Trim detailspanel is visible.");
        wxString description = VideoClip(0,3)->getDescription();
        TimelineBeginTrim(LeftCenter(VideoClip(0,3)), false);
        TimelineMoveRight(10);
        gui::timeline::DetailsTrim* detailstrim = dynamic_cast<gui::timeline::DetailsTrim*>(getTimeline().getDetails().getCurrent());
        ASSERT_NONZERO(detailstrim);
        StartTest("Trim: After the trim is done, the Clip detailspanel is visible.");
        TimelineEndTrim(false);
        gui::timeline::DetailsClip* detailsclip = dynamic_cast<gui::timeline::DetailsClip*>(getTimeline().getDetails().getCurrent());
        ASSERT_NONZERO(detailsclip);
        ASSERT_EQUALS(detailsclip->getClip()->getDescription(),description);
        Undo();
    }
    {
        StartTest("Trim: Right: During the trim, the Trim detailspanel is visible.");
        wxString description = VideoClip(0,3)->getDescription();
        TimelineBeginTrim(RightCenter(VideoClip(0,3)), false);
        TimelineMoveLeft(10);
        gui::timeline::DetailsTrim* detailstrim = dynamic_cast<gui::timeline::DetailsTrim*>(getTimeline().getDetails().getCurrent());
        ASSERT_NONZERO(detailstrim);
        StartTest("Trim: After pressing escape, the Clip detailspanel is visible.");
        TimelineKeyPress(WXK_ESCAPE);
        gui::timeline::DetailsClip* detailsclip = dynamic_cast<gui::timeline::DetailsClip*>(getTimeline().getDetails().getCurrent());
        ASSERT_NONZERO(detailsclip);
        ASSERT_EQUALS(detailsclip->getClip()->getDescription(),description);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(),mProjectFixture.OriginalLengthOfVideoClip(0,3));
        TimelineLeftUp();
    }
}

void TestTimeline::testTrimmingWithOtherTracks()
{
    StartTestSuite();
    TimelineZoomIn(2);
    WindowTriggerMenu(ID_ADDVIDEOTRACK);
    WindowTriggerMenu(ID_ADDAUDIOTRACK);
    {
        StartTest("Trim: TimelineEndTrim: Enlarge the last clip in a track (there is no empty clip after it anymore)");
        TimelineTrimRight(VideoClip(0,3),-40,false);
        pts length = VideoClip(0,3)->getLength();
        ASSERT(VideoClip(0,3)->isA<model::VideoClip>());
        TimelineDrag(From(Center(VideoClip(0,3))).AlignLeft(RightPixel(VideoClip(0,7))));
        TimelineTrimRight(VideoClip(0,7),20,false);
        ASSERT_MORE_THAN(VideoClip(0,7)->getLength(), length);
        Undo(3);
    }
    {
        StartTest("TimelineShiftTrim: TimelineBeginTrim: Shorten: with an empty other track.");
        pts previouslength = VideoClip(0,4)->getLength();
        TimelineTrimLeft(VideoClip(0,4),100);
        ASSERT_LESS_THAN_ZERO(VideoClip(0,4)->getMinAdjustBegin());
        ASSERT_MORE_THAN_ZERO(VideoClip(0,4)->getMaxAdjustBegin());
        ASSERT_LESS_THAN(VideoClip(0,4)->getLength(),previouslength);
        previouslength = VideoClip(0,4)->getLength();
    }
    {
        StartTest("TimelineShiftTrim: TimelineBeginTrim: Enlarge: with an empty other track.");
        pts previouslength = VideoClip(0,4)->getLength();
        TimelineTrimLeft(VideoClip(0,4),-10);
        ASSERT_LESS_THAN_ZERO(VideoClip(0,4)->getMinAdjustBegin());
        ASSERT_MORE_THAN_ZERO(VideoClip(0,4)->getMaxAdjustBegin());
        ASSERT_MORE_THAN(VideoClip(0,4)->getLength(),previouslength);
    }
    {
        StartTest("TimelineShiftTrim: TimelineEndTrim: Shorten: with an empty other track.");
        pts previouslength = VideoClip(0,4)->getLength();
        TimelineTrimRight(VideoClip(0,4),-20);
        ASSERT_LESS_THAN_ZERO(VideoClip(0,4)->getMinAdjustEnd());
        ASSERT_MORE_THAN_ZERO(VideoClip(0,4)->getMaxAdjustEnd());
        ASSERT_LESS_THAN(VideoClip(0,4)->getLength(),previouslength);
    }
    {
        StartTest("TimelineShiftTrim: TimelineEndTrim: Enlarge: with an empty other track.");
        pts previouslength = VideoClip(0,4)->getLength();
        TimelineTrimRight(VideoClip(0,4),10);
        ASSERT_LESS_THAN_ZERO(VideoClip(0,4)->getMinAdjustEnd());
        ASSERT_MORE_THAN_ZERO(VideoClip(0,4)->getMaxAdjustEnd());
        ASSERT_MORE_THAN(VideoClip(0,4)->getLength(),previouslength);
    }
    {
        StartTest("Make a clip in another track (preparation).");
        TimelineDragToTrack(1,VideoClip(0,3),AudioClip(0,3));
        ASSERT_VIDEOTRACK1(EmptyClip)                      (VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip)(AudioClip);
        ASSERT_AUDIOTRACK1(EmptyClip)                      (AudioClip);
        ASSERT_LESS_THAN_ZERO(VideoClip(0,4)->getMinAdjustBegin());
        ASSERT_MORE_THAN_ZERO(VideoClip(0,4)->getMaxAdjustBegin());
        ASSERT_LESS_THAN_ZERO(VideoClip(0,4)->getMinAdjustEnd());
        ASSERT_MORE_THAN_ZERO(VideoClip(0,4)->getMaxAdjustEnd());
        ASSERT_LESS_THAN(VideoTrack(1)->getLength(),VideoTrack(0)->getLength());
        ASSERT_LESS_THAN(AudioTrack(1)->getLength(),AudioTrack(0)->getLength());
        ASSERT_EQUALS(VideoClip(1,1)->getLeftPts(),VideoClip(0,3)->getLeftPts());
        ASSERT_EQUALS(AudioClip(1,1)->getLeftPts(),AudioClip(0,3)->getLeftPts());
        ASSERT_EQUALS(VideoClip(1,1)->getRightPts(),VideoClip(0,4)->getLeftPts());
        ASSERT_EQUALS(AudioClip(1,1)->getRightPts(),AudioClip(0,4)->getLeftPts());
    }
    {
        StartTest("Move the 'to be tested' clip's left point inbetween the clip in the other track (preparation).");
        ASSERT(!getTimeline().getKeyboard().getShiftDown());
        TimelineDrag(From(Center(VideoClip(0,4))).AlignLeft(HCenter(VideoClip(0,3))));
        ASSERT_LESS_THAN(VideoTrack(1)->getLength(),VideoTrack(0)->getLength());
        ASSERT_LESS_THAN(AudioTrack(1)->getLength(),AudioTrack(0)->getLength());
        ASSERT(!getTimeline().getKeyboard().getShiftDown());
        ASSERT_MORE_THAN(VideoClip(0,4)->getLeftPts(),VideoClip(1,1)->getLeftPts());
        ASSERT_LESS_THAN(VideoClip(0,4)->getLeftPts(),VideoClip(1,1)->getRightPts());
    }
    {
        StartTest("TimelineShiftTrim: TimelineEndTrim: Shorten: with another track that is shorter than the trim position (this imposes a lower bound on the shift).");
        pts diff = VideoClip(0,4)->getRightPts() - VideoTrack(1)->getLength();
        pts track0len = VideoTrack(0)->getLength();
        TimelineTrimRight(VideoClip(0,4),-200);
        ASSERT_LESS_THAN(VideoTrack(1)->getLength(),VideoTrack(0)->getLength());
        ASSERT_LESS_THAN(AudioTrack(1)->getLength(),AudioTrack(0)->getLength());
        ASSERT_EQUALS(VideoClip(0,4)->getRightPts(),VideoTrack(1)->getLength());
        ASSERT_EQUALS(VideoTrack(0)->getLength(),track0len - diff);
        Undo(); // Undo the trim
    }
    pts previouslength = VideoClip(0,4)->getLength();
    {
        StartTest("TimelineShiftTrim: TimelineBeginTrim: Shorten: with another track that has a clip on the trim position (no trim possible).");
        TimelineTrimLeft(VideoClip(0,4),-100);
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), previouslength);
        StartTest("TimelineShiftTrim: TimelineBeginTrim: Enlarge: with another track that has a clip on the trim position (no trim possible).");
        TimelineTrimLeft(VideoClip(0,4),+100);
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), previouslength);
    }
    {
        StartTest("Move the clip in the other track over the end of the tested clip (preparation).");
        ASSERT(!getTimeline().getKeyboard().getShiftDown());
        TimelineDrag(From(Center(VideoClip(1,1))).To(wxPoint(RightPixel(VideoClip(0,4)),VCenter(VideoTrack(1)))));
        ASSERT(!getTimeline().getKeyboard().getShiftDown());
        ASSERT_VIDEOTRACK1(EmptyClip)(VideoClip);
        ASSERT_VIDEOTRACK1SIZE(2);
    }
    {
        StartTest("TimelineShiftTrim: TimelineEndTrim: Shorten: with another track that has a clip on the trim position (no trim possible).");
        TimelineTrimRight(VideoClip(0,4),-100);
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), previouslength);
        StartTest("TimelineShiftTrim: TimelineEndTrim: Enlarge: with another track that has a clip on the trim position (no trim possible).");
        TimelineTrimRight(VideoClip(0,4),100);
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), previouslength);
    }
    {
        StartTest("Ensure that there's a clip in another track before AND after AND 'inbetween' the clip under test (preparation).");
        Undo(2);
        TimelineDragToTrack(1,VideoClip(0,5),AudioClip(0,5));
        TimelineTrimRight(VideoClip(0,6),-40);
    }
    {
        StartTest("TimelineShiftTrim: Put clips in other tracks 'around' trim points but not exactly ON the trim point so that trimming is possible (Preparation).");
        ASSERT(!getTimeline().getKeyboard().getShiftDown());
        ASSERT_VIDEOTRACK1(EmptyClip)                      (VideoClip)(EmptyClip)(VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(VideoClip)(EmptyClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip)(EmptyClip)(AudioClip);
        ASSERT_AUDIOTRACK1(EmptyClip)                      (AudioClip)(EmptyClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(1,3)->getRightPts(),VideoClip(0,6)->getLeftPts());
        ASSERT_EQUALS(AudioClip(1,3)->getRightPts(),AudioClip(0,6)->getLeftPts());
        TimelineDragToTrack(1,VideoClip(0,6),AudioClip(0,6));
        ASSERT(!getTimeline().getKeyboard().getShiftDown());
        ASSERT_VIDEOTRACK1(EmptyClip)                      (VideoClip)(EmptyClip)(VideoClip)(VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip);
        ASSERT_AUDIOTRACK1(EmptyClip)                      (AudioClip)(EmptyClip)(AudioClip)(AudioClip);
        ASSERT_VIDEOTRACK0SIZE(5); // Check that emptyness at end is removed
        ASSERT_AUDIOTRACK0SIZE(5); // Check that emptyness at end is removed
        ASSERT_EQUALS(VideoClip(1,1)->getLeftPts(),VideoClip(0,3)->getLeftPts());
        ASSERT_EQUALS(AudioClip(1,1)->getLeftPts(),AudioClip(0,3)->getLeftPts());
        ASSERT_EQUALS(VideoClip(1,3)->getLeftPts(),VideoClip(0,4)->getRightPts());
        ASSERT_EQUALS(AudioClip(1,3)->getLeftPts(),AudioClip(0,4)->getRightPts());
        TimelineDrag(From(Center(VideoClip(1,1))).To(Center(VideoClip(1,1))-wxPoint(8,0)));
        ASSERT(!getTimeline().getKeyboard().getShiftDown());
        ASSERT_VIDEOTRACK1(EmptyClip)(VideoClip)(EmptyClip)(VideoClip)(VideoClip);
        TimelineDrag(From(Center(VideoClip(1,3))).To(Center(VideoClip(1,3))+wxPoint(8,0)));
        ASSERT(!getTimeline().getKeyboard().getShiftDown());
        ASSERT_VIDEOTRACK1(EmptyClip)(VideoClip)(EmptyClip)(VideoClip)(VideoClip);
        TimelineDrag(From(Center(VideoClip(1,4))).AlignLeft(LeftPixel(VideoClip(0,4))+20));
        ASSERT_VIDEOTRACK1(EmptyClip)                   (VideoClip)(EmptyClip)(VideoClip)(EmptyClip)(VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(       VideoClip      );
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(       AudioClip      );
        ASSERT_AUDIOTRACK1(EmptyClip)                   (AudioClip)(EmptyClip)(AudioClip)(EmptyClip)(AudioClip);
        ASSERT_VIDEOTRACK0SIZE(5); // Check that emptyness at end is removed
        ASSERT_AUDIOTRACK0SIZE(5); // Check that emptyness at end is removed
    }
    previouslength = VideoClip(0,4)->getLength();
    pts minbegin = VideoClip(0,4)->getMinAdjustBegin();
    pts maxbegin = VideoClip(0,4)->getMaxAdjustBegin();
    pts minend = VideoClip(0,4)->getMinAdjustEnd();
    pts maxend = VideoClip(0,4)->getMaxAdjustEnd();
    {
        StartTest("TimelineShiftTrim: TimelineBeginTrim: Shorten: other track space imposes a trim restriction.");
        pts maxadjust = VideoClip(1,3)->getLeftPts() - VideoClip(0,4)->getLeftPts();
        TimelineTrimLeft(VideoClip(0,4),200);
        ASSERT_MORE_THAN(VideoClip(0,4)->getLength(), previouslength - maxbegin); // Can't trim to the max due to the restriction in the other track
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), previouslength - maxadjust);
        ASSERT_EQUALS(VideoClip(0,4)->getLeftPts(),VideoClip(1,3)->getLeftPts());
        Undo();
    }
    {
        StartTest("TimelineShiftTrim: TimelineBeginTrim: Enlarge: other track space imposes NO trim restriction.");
        TimelineTrimLeft(VideoClip(0,4),-200);
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), previouslength - minbegin); // Note: minbegin < 0
        Undo();
    }
    {
        StartTest("TimelineShiftTrim: TimelineEndTrim: Shorten: other track space imposes a trim restriction.");
        pts minadjust = VideoClip(1,3)->getRightPts() - VideoClip(0,4)->getRightPts();
        TimelineTrimRight(VideoClip(0,4),-200);
        ASSERT_MORE_THAN(VideoClip(0,4)->getLength(), previouslength + minend); // Can't trim to the max due to the restriction in the other track
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), previouslength + minadjust ); // Note: minadjust < 0
        ASSERT_EQUALS(VideoClip(0,4)->getRightPts(),VideoClip(1,3)->getRightPts());
        Undo();
    }
    {
        StartTest("TimelineShiftTrim: TimelineEndTrim: Enlarge: other track space imposes NO trim restriction.");
        TimelineTrimRight(VideoClip(0,4),200);
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), previouslength + maxend);
        Undo();
    }
    {
        StartTest("Trim: Snap to clip.");
        Undo(11); // Undo until only the add track has been done
        // NOTE: At this point the scrolling has been changed. Trimming and snapping with a scroll offset is thus also tested.
        ConfigFixture.SnapToClips(true).SnapToCursor(false);
        TimelineTrimLeft(VideoClip(0,4),40,false);
        TimelineDrag(From(Center(VideoClip(0,6))).To(wxPoint(HCenter(VideoClip(0,4)),VCenter(getSequence()->getVideoTrack(1)))));
        ASSERT_EQUALS(VideoClip(0,5)->getLink(),AudioClip(0,5));
        ASSERT_EQUALS(VideoClip(1,1)->getLink(),AudioClip(0,4));
        TimelineTrimLeft(VideoClip(0,5),10,false,false);
        TimelineMove(wxPoint(RightPixel(VideoClip(1,1))-26,VCenter(VideoClip(0,5))));
        TimelineEndTrim(false);
        ASSERT_DIFFERS(VideoClip(0,5)->getLeftPts(),VideoClip(1,1)->getRightPts());
        Undo();
        TimelineTrimLeft(VideoClip(0,5),10,false,false);
        TimelineMove(wxPoint(RightPixel(VideoClip(1,1))-6,VCenter(VideoClip(0,5))));
        TimelineEndTrim(false);
        ASSERT_EQUALS(VideoClip(0,5)->getLeftPts(),VideoClip(1,1)->getRightPts());
        Undo(3);
    }
}

void TestTimeline::testShowDebugInfo()
{
    StartTestSuite();
    model::SequencePtr sequence = getSequence();
    Config::setShowDebugInfo(true);
    WindowTriggerMenu(ID_CLOSESEQUENCE);
    ProjectViewOpenTimelineForSequence(sequence);
    Config::setShowDebugInfo(false);
}

void TestTimeline::testPositionCursor()
{
    StartTestSuite();
    TimelineZoomIn(6);
    TimelinePositionCursor(LeftPixel(VideoClip(0, 4)));
    ASSERT_DIFFERS(getTimeline().getScrolling().getFourthPts(), VideoClip(0, 4)->getLeftPts());
    TimelineKeyPress('v');
    ASSERT_EQUALS(getTimeline().getScrolling().getFourthPts(), VideoClip(0, 4)->getLeftPts());
}


void TestTimeline::testRemoveEmptyClip()
{
    StartTestSuite();
    TimelineLeftClick(Center(VideoClip(0, 1)));
    TimelineKeyPress(WXK_DELETE);
    ASSERT_SELECTION_SIZE(0);
    ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip);
    ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip);
    TimelineMove(Center(VideoClip(0, 1)));
    TimelineKeyPress('c');
    ASSERT_VIDEOTRACK0(VideoClip)(VideoClip);
    ASSERT_AUDIOTRACK0(AudioClip)(AudioClip);
    ASSERT_EQUALS(VideoClip(0, 1)->getLeftPts(), mProjectFixture.OriginalLengthOfVideoClip(0,0));
    ASSERT_EQUALS(VideoClip(0, 1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
    ASSERT_EQUALS(AudioClip(0, 1)->getLeftPts(), mProjectFixture.OriginalLengthOfAudioClip(0,0));
    ASSERT_EQUALS(AudioClip(0, 1)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,2));
    Undo();
    ASSERT_VIDEOTRACK0(VideoClip)(EmptyClip)(VideoClip);
    ASSERT_AUDIOTRACK0(AudioClip)(EmptyClip)(AudioClip);
}

} // namespace