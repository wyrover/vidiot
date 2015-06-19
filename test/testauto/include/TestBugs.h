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

#ifndef TEST_BUGS_H
#define TEST_BUGS_H

#include "TestAuto.h"

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

    /// See [#172]
    /// Trimming a clip that has both an in- and an out-transition caused a crash, because the UI
    /// was already updating while the clip edit was only partially done. First, a clip is removed,
    /// then, its replacement is added. However, after the removal an illegal intermediary state
    /// could be present. In the bug case, the UI update was at that exact moment, causing a crash
    /// in ClipView (check for a transition for which the previous clip was an in-transition, or
    /// a transition for which the next clip was an out-transition - both are impossible).
    void testTrimClipInbetweenTransitionsCausesCrash();

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

    /// Had a crash in the soundtouch handling of small chunk sizes (< 4 samples).
    void testPlaybackWithMultipleAudioTracks();

    /// See [#132]. A crash occurred when pressing shift-delete in case there were multiple
    /// (video, in this case) tracks, and the deleted clips' position was larger than the 
    /// length of one of the other tracks.
    void testCrashOnShiftDeleteWithMultipleTracks();

    /// See [#133]. Crash occurred when doing the following:
    /// Make transition between two clips such that the transition is completely to the right of
    /// the cut between the two clips. When starting a trim (via the mouse) on the left edge 
    /// of the transition, the crash occurred.
    void testCrashWhenTrimmingWithTransitionOnOneSideOfCut();

    /// See [#133]. After fixing the crash, trimming does not work when starting the trim on
    /// the 'other' side of the transiton.
    void testTrimmingWithTransitionOnOneSideOfCut();

    /// See [#126]. Snap to cursor caused the dragged clip's left position to be moved to a 
    /// position < 0, resulting in assert.
    void testSnapClipBeforeBeginOfTimeline();

    /// See [#141] and [#142]
    /// Some of the asserts in determineClipBounds caused a crash. Reason was not taking into 
    /// account scenarios with both transitions AND linked clips with different sizes.
    /// Note that the crash only occurred when the faulty clip was selected, causing its 
    /// details to be shown.
    void testCrashWhenDeterminingClipSizeBoundsForLinkedClipsWithDifferentLengthAndOutTransition();

    /// It was possible to trigger the creation of an in- (or out-) crossfade for a clip twice.
    /// The second creation of the transition resulted in the crash.
    void testCrashWhenCreatingCrossfadeViaKeyboardTwice();

    /// See [#155]
    /// Creating a audio transition at the end of a clip (at the end of the underlying file)
    /// caused the transition to have an illegal length: it overlapped the cut between the
    /// clips (12 to the left, 12 to the right), which is different than what happened for
    /// creating the same transition in the video track: in that case the transition was
    /// fully to the left of the cut (12 to the left only).
    ///
    /// This bug was caused by a clip's file using uninitialized data (mLength) during
    /// the creation of a transition. The uninitialized data was caused by loading a
    /// timeline (causing the files not to be opened yet) and then creating a transition,
    /// using data from the not-yet-opened (thus, meta data unknown) file.
    void testCrashCausedByCreatingTransitionAtAudioClipEndAfterReadingProjectFromDisk();

    /// See [#164]
    /// If a audio+video clip has an out transition on either audio OR video, then the
    /// snapping behaviour was different between starting the trim on either audio OR video.
    /// When trimming was started on the part without a transition, then snapping behaviour
    /// was ok. When starting the trim on the part with the transition (thus, starting the
    /// trim while hovering over the end of the transition) snapping caused the clip to
    /// be extended to the end of the transition immediately. Thus, without really moving
    /// the mouse the clip was already extended (to the length of the transition).
    ///
    /// Note that this behaviour was only seen with out-transitions that were part of
    /// a project that was ready from disk. For newly created transitions, the behaviour 
    /// was not seen.
    void testEndTrimAtOutTransitionInSavedDocumentEndCausesSnappingProblemVideo();

    /// See testEndTrimAtOutTransitionInSavedDocumentEndCausesSnappingProblemVideo()
    void testEndTrimAtOutTransitionInSavedDocumentEndCausesSnappingProblemAudio();

    /// See [#177]
    /// Was unable to trim and extend a clip when both the clip and its link had out transitions,
    /// even though there was enough empty space after both of these transitions.
    void testTrimAndExtendVideoAndAudioClipsThatBothHaveOutTransitions();

    /// See [#209]
    /// Unlink audio clip from its video counterpart, and delete the video part.
    /// Then shift-trim to extend the audio clip, enlarging it beyond the file length.
    /// Caused crash.
    void testCrashWhenEnlargingUnlinkedAudioClipBeyondFileLength();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    FixtureProject mProjectFixture;
};

}

using namespace test;

#endif