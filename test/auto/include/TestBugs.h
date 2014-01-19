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

#ifndef TEST_BUGS_H
#define TEST_BUGS_H

namespace test
{

/// This class tests scenarios that were problematic at one point.
class TestBugs : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestBugs>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    virtual void setUp();       ///< Called before each test.
    virtual void tearDown();    ///< Called after each test.

    //////////////////////////////////////////////////////////////////////////
    // TEST CASES
    //////////////////////////////////////////////////////////////////////////

    /// Video decoding was not properly initialized in all cases. Particularly, when moving
    /// to one of the last frames (thus, near EOF) and then starting the decode, it was possible
    /// that the reading of all packets had already been finished (eof set for the video file)
    /// and a subsequent call to startDecoding* was returned immediately because of the EOF flag.
    /// Bug was fixed by only not starting a renewed 'reading of packets'. Decoding would be started,
    /// regardless the EOF flag.
    void testVideoDecodingError();

    /// A hangup occurred after resetting the details view multiple times. Apparently, calling
    /// DestroyChildren too often seemed to (note: seemed to) cause this.
    void testHangupAfterResettingDetailsView();

    /// The details view was sometimes not shown after moving the cursor, and then clicking
    /// on a clip.
    void testDetailsNotShownAfterMovingTimelineCursor();

    /// A bug in the linking algorithm was exposed by an assert triggered when trimming after
    /// dropping a clip on the beginning of a linked clip. The linking algorithm caused clip A
    /// to be first replaced with clips B and C. Then, clip B was simply removed from the track,
    /// instead of being replaced with an empty list. That caused (in the end) the clip originally
    /// linked to A not being linked to C.
    void testLinkingErrorWhenDroppingOverBeginOfLinkedClip();

    /// Get next for EmptyClips kept on returning empty frames, too often. This was first seen
    /// when making a composition in which the clips in the second/third track were shown too
    /// late because of the bug.
    /// The playback in this test should first show one frame, then two smaller frames on top of it.
    void testErrorInGetNextHandlingForEmptyClips();

    /// It's sometimes possible to start a drag operation without having something selected. Particular
    /// example: select something, then unselect by pressing the mouse, but not releasing it, then
    /// immediately start dragging.
    void testDraggingWithoutSelection();

    /// All tests with a long timeline are combined, to avoid reopening the large timeline more than once.
    ///
    /// Bug 1:
    /// Due to excessive repainting the timeline playback caused a 'sorta' hangup: playback continued
    /// (audio kept on playing) but the GUI remained locked.
    /// Caused by ClipView constantly triggering a 'invalidateBitmap' because of debug information
    /// being updated (particularly, the rendering progress feedback was updated continuously).
    /// This happened particularly with a long timeline, fully zoomed in.
    ///
    /// Bug 2:
    /// When the sequence was playing, pressing shift (start interval) caused playback to continue
    /// indefinitely.
    void testBugsWithLongTimeline();

    /// Due to a integer calculation bug, playback of an empty clip resulting in the clip AFTER
    /// the empty clip being played almost immediately. This was caused by getNextAudio returning
    /// '0' (thus, end of clip) immediately, after which the next clip was played.
    void testPlaybackEmptyClip();

    /// A clip can have length 0 if it is used for a transition, and is fully 'obscured' by the
    /// transition (thus, it only provides frames to the transition, and provides no frames to the
    /// track itselves). In the trimming code, an 'adjacent' clip is determined for the preview
    /// operation. In the particular case that a 0 length clip is the adjacent clip, the transition
    /// must be used for obtaining the 'previewed adjacent frame'.
    void testTrimmingClipAdjacentToZeroLengthClipUsedForTransition();

    /// Create two crossfades 'around' a video clip. Rightclick on the clip. Select delete and trim.
    /// Both the transitions AND the clip are deleted. That causes the audio and video tracks to become misaligned.
    /// Note: This also tests explicitly adding a transition (type) twice in the same sequence (bug once)
    void testDeleteClipInbetweenTransitionsCausesTimelineMessUp();

    /// Due to operations in multiple tracks, the first operation removed some of the clips that were
    /// required for the operations in the second track. Since these clips had already been removed
    /// a crash occurred (check that replacing a clip in AClipEdit is only allowed if a clip is still
    /// part of a track).
    void testCrashWhenDroppingPartiallyOverATransition();

    /// If a clip in 'another' track touches a clip that is being trimmed, at the right side,
    /// then trimming is not allowed. If the clip is in the same track as the trimmed clip,
    /// then trimming IS allowed.
    void testShiftTrimNotAllowedWithAdjacentClipInOtherTrack();

    /// Add a clip to a folder, then remove it again (Undo). Then it's removed from disk
    /// and added again (to the same folder) via Redo. After creating a sequence from
    /// the mentioned folder, a crash occurred in the clip view, using a maxint size
    /// for its required bitmap.
    void testAddNonexistentFileViaRedo();

    /// Add a clip to a folder, then remove it again (Delete). Then it's removed from disk
    /// and added again (to the same folder) via Undo. After creating a sequence from
    /// the mentioned folder, a crash occurred in the clip view, using a maxint size
    /// for its required bitmap.
    void testAddNonexistentFileViaUndo();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    FixtureProject mProjectFixture;
};

}

using namespace test;

#endif // TEST_BUGS_H