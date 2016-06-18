// Copyright 2015-2016 Eric Raijmakers.
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

#include "TestClipSpeed.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestClipSpeed::setUp()
{
    mProjectFixture.init();
}

void TestClipSpeed::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestClipSpeed::testChangeClipSpeed()
{
    StartTestSuite();
    TimelineZoomIn(2);
    {
        StartTest("Allowed for linked clips");
        TimelineLeftClick(Center(VideoClip(0, 4)));
        ASSERT(DetailsView(VideoClip(0, 4)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
    }
    Unlink(VideoClip(0, 4));
    TimelineSelectClips({});
    {
        StartTest("Not allowed for unlinked video if an audio clip is present in the same time frame");
        TimelineLeftClick(Center(VideoClip(0, 4)));
        ASSERT(DetailsView(VideoClip(0, 4)));
        ASSERT(!DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(!DetailsClipView()->getSpeedSpin()->IsEnabled());
    }
    {
        StartTest("Not allowed for unlinked audio if a video clip is present in the same time frame");
        TimelineLeftClick(Center(AudioClip(0, 4)));
        ASSERT(DetailsView(AudioClip(0, 4)));
        ASSERT(!DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(!DetailsClipView()->getSpeedSpin()->IsEnabled());
    }
    {
        StartTest("Allowed for unlinked video");
        TimelineDeleteClip(AudioClip(0, 4));
        TimelineLeftClick(Center(VideoClip(0, 4)));
        ASSERT(DetailsView(VideoClip(0, 4)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        Undo();
    }
    {
        StartTest("Allowed for unlinked audio");
        TimelineDeleteClip(VideoClip(0, 4));
        TimelineLeftClick(Center(AudioClip(0, 4)));
        ASSERT(DetailsView(AudioClip(0, 4)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        Undo();
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////
    {
        StartTest("Increase (size reduction) not allowed when unlinked audio clip in other track (partial on begin)");
        TimelineSelectClips({});
        TimelineDrag(From(Center(AudioClip(0, 4))).To(LeftCenter(AudioClip(0, 4))));
        TimelineLeftClick(Center(VideoClip(0, 4)));
        ASSERT(DetailsView(VideoClip(0, 4)));
        ASSERT(!DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(!DetailsClipView()->getSpeedSpin()->IsEnabled());
        Undo();
    }
    {
        StartTest("Increase (size reduction) not allowed when unlinked audio clip in other track (partial on end)");
        TimelineSelectClips({});
        TimelineDrag(From(Center(AudioClip(0, 4))).To(RightCenter(AudioClip(0, 4))));
        TimelineLeftClick(Center(VideoClip(0, 4)));
        ASSERT(DetailsView(VideoClip(0, 4)));
        ASSERT(!DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(!DetailsClipView()->getSpeedSpin()->IsEnabled());
        Undo();
    }
    {
        StartTest("Increase (size reduction) not allowed when unlinked audio clip in other track (partial on begin)");
        TimelineSelectClips({});
        TimelineDrag(From(Center(VideoClip(0, 4))).To(LeftCenter(VideoClip(0, 4))));
        TimelineLeftClick(Center(AudioClip(0, 4)));
        ASSERT(DetailsView(AudioClip(0, 4)));
        ASSERT(!DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(!DetailsClipView()->getSpeedSpin()->IsEnabled());
        Undo();
    }
    {
        StartTest("Increase (size reduction) not allowed when unlinked audio clip in other track (partial on end)");
        TimelineSelectClips({});
        TimelineDrag(From(Center(VideoClip(0, 4))).To(RightCenter(VideoClip(0, 4))));
        TimelineLeftClick(Center(AudioClip(0, 4)));
        ASSERT(DetailsView(AudioClip(0, 4)));
        ASSERT(!DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(!DetailsClipView()->getSpeedSpin()->IsEnabled());
        Undo();
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////
    {
        StartTest("Increase (size reduction) allowed when empty clips in other tracks");
        TimelineDeleteClip(AudioClip(0, 4));
        TimelineLeftClick(Center(VideoClip(0, 4)));
        ASSERT(DetailsView(VideoClip(0, 4)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 14900);
        ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(50, 1));
        ASSERT_EQUALS(DetailsClipView()->getSpeedSpin()->GetValue(), 50.0);
        ASSERT_EQUALS(VideoClip(0, 6)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
        Undo(2);
    }
    {
        StartTest("Increase (size reduction) allowed when no clips in other tracks");
        TimelineDeleteClips({ AudioClip(0, 0), AudioClip(0, 1), AudioClip(0, 2), AudioClip(0, 3), AudioClip(0, 4), AudioClip(0, 5), AudioClip(0, 6) });
        ASSERT_AUDIOTRACK0SIZE(0);
        TimelineLeftClick(Center(VideoClip(0, 1)));
        ASSERT(DetailsView(VideoClip(0, 1)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 14900);
        ASSERT_EQUALS(DetailsClipView()->getSpeedSpin()->GetValue(), 50.0);
        ASSERT_EQUALS(DetailsClipView()->getSpeedSlider()->GetValue(), 14900);
        ASSERT_CLIP_SPEED(VideoClip(0, 1), rational64(50, 1));
        Undo(2);
    }
    {
        StartTest("Increase (size reduction) allowed when empty clips in other tracks");
        TimelineDeleteClip(VideoClip(0, 4));
        TimelineLeftClick(Center(AudioClip(0, 4)));
        ASSERT(DetailsView(AudioClip(0, 4)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 14900);
        ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(50, 1));
        ASSERT_EQUALS(DetailsClipView()->getSpeedSpin()->GetValue(), 50.0);
        ASSERT_EQUALS(AudioClip(0, 6)->getLeftPts(), VideoClip(0, 6)->getLeftPts()); // Verify proper shifting
        Undo(2);
    }
    {
        StartTest("Increase (size reduction) allowed when no clips in other tracks");
        TimelineDeleteClips({ VideoClip(0, 0), VideoClip(0, 1), VideoClip(0, 2), VideoClip(0, 3), VideoClip(0, 4), VideoClip(0, 5), VideoClip(0, 6) });
        ASSERT_VIDEOTRACK0SIZE(0);
        TimelineLeftClick(Center(AudioClip(0, 1)));
        ASSERT(DetailsView(AudioClip(0, 1)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 14900);
        ASSERT_EQUALS(DetailsClipView()->getSpeedSpin()->GetValue(), 50.0);
        ASSERT_EQUALS(DetailsClipView()->getSpeedSlider()->GetValue(), 14900);
        ASSERT_CLIP_SPEED(AudioClip(0, 1), rational64(50, 1));
        Undo(2);
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////
    {
        StartTest("Increase (size reduction) not allowed with empty clips in other tracks, with enough length, but shifted.");
        Unlink(VideoClip(0, 3));
        Unlink(VideoClip(0, 5));
        TimelineDeleteClip(AudioClip(0, 4));
        TimelineDrag(From(Center(AudioClip(0, 3))).MoveLeft(20));
        TimelineDrag(From(Center(AudioClip(0, 5))).MoveLeft(20)); // Now the empty clip 'under' the to-be-changed clip has a large enough length, but does not fully cover the clip timeframe.
        TimelineLeftClick(Center(VideoClip(0, 4)));
        ASSERT(DetailsView(VideoClip(0, 4)));
        ASSERT(!DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(!DetailsClipView()->getSpeedSpin()->IsEnabled());
        Undo(5);
        ASSERT_EQUALS(VideoClip(0, 6)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
    }
    {
        StartTest("Decrease (size enlargement) not allowed with empty clips in other tracks, with enough length, but shifted.");
        Unlink(VideoClip(0, 3));
        Unlink(VideoClip(0, 5));
        TimelineDeleteClip(AudioClip(0, 4));
        TimelineDrag(From(Center(AudioClip(0, 3))).MoveLeft(20));
        TimelineDrag(From(Center(AudioClip(0, 5))).MoveLeft(20)); // Now the empty clip 'under' the to-be-changed clip has a large enough length, but does not fully cover the clip timeframe.
        TimelineLeftClick(Center(VideoClip(0, 4)));
        ASSERT(DetailsView(VideoClip(0, 4)));
        ASSERT(!DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(!DetailsClipView()->getSpeedSpin()->IsEnabled());
        Undo(5);
        ASSERT_EQUALS(VideoClip(0, 6)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
    }
    {
        StartTest("Increase (size reduction) not allowed with empty clips in other tracks, with enough length, but shifted.");
        Unlink(AudioClip(0, 3));
        Unlink(AudioClip(0, 5));
        TimelineDeleteClip(VideoClip(0, 4));
        TimelineDrag(From(Center(VideoClip(0, 3))).MoveLeft(20));
        TimelineDrag(From(Center(VideoClip(0, 5))).MoveLeft(20)); // Now the empty clip 'under' the to-be-changed clip has a large enough length, but does not fully cover the clip timeframe.
        TimelineLeftClick(Center(AudioClip(0, 4)));
        ASSERT(DetailsView(AudioClip(0, 4)));
        ASSERT(!DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(!DetailsClipView()->getSpeedSpin()->IsEnabled());
        Undo(5);
        ASSERT_EQUALS(AudioClip(0, 6)->getLeftPts(), VideoClip(0, 6)->getLeftPts()); // Verify proper shifting
    }
    {
        StartTest("Decrease (size enlargement) not allowed with empty clips in other tracks, with enough length, but shifted.");
        Unlink(AudioClip(0, 3));
        Unlink(AudioClip(0, 5));
        TimelineDeleteClip(VideoClip(0, 4));
        TimelineDrag(From(Center(VideoClip(0, 3))).MoveLeft(20));
        TimelineDrag(From(Center(VideoClip(0, 5))).MoveLeft(20)); // Now the empty clip 'under' the to-be-changed clip has a large enough length, but does not fully cover the clip timeframe.
        TimelineLeftClick(Center(AudioClip(0, 4)));
        ASSERT(DetailsView(AudioClip(0, 4)));
        ASSERT(!DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(!DetailsClipView()->getSpeedSpin()->IsEnabled());
        Undo(5);
        ASSERT_EQUALS(AudioClip(0, 6)->getLeftPts(), VideoClip(0, 6)->getLeftPts()); // Verify proper shifting
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    {
        StartTest("Change spin: Decrease (size enlargement)");
        TimelineDeleteClips({ AudioClip(0, 0), AudioClip(0, 1), AudioClip(0, 2), AudioClip(0, 3), AudioClip(0, 4), AudioClip(0, 5), AudioClip(0, 6) });
        ASSERT_AUDIOTRACK0SIZE(0);
        TimelineLeftClick(Center(VideoClip(0, 1)));
        ASSERT(DetailsView(VideoClip(0, 1)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSpin(), 0.5);
        ASSERT_CLIP_SPEED(VideoClip(0, 1), rational64(1, 2));
        ASSERT_EQUALS(DetailsClipView()->getSpeedSlider()->GetValue(), 5000);
        ASSERT_EQUALS(DetailsClipView()->getSpeedSpin()->GetValue(), 0.5);
        Undo(2);
    }
    {
        StartTest("Change spin: Increase (size reduction)");
        TimelineDeleteClips({ AudioClip(0, 0), AudioClip(0, 1), AudioClip(0, 2), AudioClip(0, 3), AudioClip(0, 4), AudioClip(0, 5), AudioClip(0, 6) });
        ASSERT_AUDIOTRACK0SIZE(0);
        TimelineLeftClick(Center(VideoClip(0, 1)));
        ASSERT(DetailsView(VideoClip(0, 1)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSpin(), 50.0);
        ASSERT_CLIP_SPEED(VideoClip(0, 1), rational64(50, 1));
        ASSERT_EQUALS(DetailsClipView()->getSpeedSlider()->GetValue(), 14900);
        ASSERT_EQUALS(DetailsClipView()->getSpeedSpin()->GetValue(), 50.0);
        Undo(2);
    }
    {
        StartTest("Change spin: Decrease (size enlargement)");
        TimelineDeleteClips({ VideoClip(0, 0), VideoClip(0, 1), VideoClip(0, 2), VideoClip(0, 3), VideoClip(0, 4), VideoClip(0, 5), VideoClip(0, 6) });
        ASSERT_VIDEOTRACK0SIZE(0);
        TimelineLeftClick(Center(AudioClip(0, 1)));
        ASSERT(DetailsView(AudioClip(0, 1)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSpin(), 0.5);
        ASSERT_CLIP_SPEED(AudioClip(0, 1), rational64(1, 2));
        ASSERT_EQUALS(DetailsClipView()->getSpeedSlider()->GetValue(), 5000);
        ASSERT_EQUALS(DetailsClipView()->getSpeedSpin()->GetValue(), 0.5);
        Undo(2);
    }
    {
        StartTest("Change spin: Increase (size reduction)");
        TimelineDeleteClips({ VideoClip(0, 0), VideoClip(0, 1), VideoClip(0, 2), VideoClip(0, 3), VideoClip(0, 4), VideoClip(0, 5), VideoClip(0, 6) });
        ASSERT_VIDEOTRACK0SIZE(0);
        TimelineLeftClick(Center(AudioClip(0, 1)));
        ASSERT(DetailsView(AudioClip(0, 1)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSpin(), 50.0);
        ASSERT_CLIP_SPEED(AudioClip(0, 1), rational64(50, 1));
        ASSERT_EQUALS(DetailsClipView()->getSpeedSlider()->GetValue(), 14900);
        ASSERT_EQUALS(DetailsClipView()->getSpeedSpin()->GetValue(), 50.0);
        Undo(2);
    }
}

void TestClipSpeed::testChangeClipSpeedWithOffsetAndLength()
{
    StartTestSuite();
    TimelineZoomIn(2);

    /////////////////////////////////////////////////////////////////////////////////////////////////
    Unlink(VideoClip(0, 4));
    TimelineDeleteClip(AudioClip(0, 4));
    TimelineTrimRight(VideoClip(0, 4), -50, false);
    TimelineTrimLeft(VideoClip(0, 4), 50, false); // Note: the trim adds an empty clip, thus the clip must now be indexed with '5'

    ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
    model::VideoFramePtr referenceFirstFrame{ FirstFrame(VideoClip(0, 5)) };
    model::VideoFramePtr referenceLastFrame{ LastFrame(VideoClip(0, 5)) };
    {
        StartTest("Unlinked Video: Speed decrease: Enlarging clip allowed, begin and end frames stay the same");
        TimelineLeftClick(Center(VideoClip(0,5)));
        ASSERT(DetailsView(VideoClip(0,5)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
        ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1,100));
        ASSERT_EQUALS(DetailsClipView()->getSpeedSpin()->GetValue(), 0.01);
        ASSERT_EQUALS(DetailsClipView()->getSpeedSlider()->GetValue(), 100);
        ASSERT_EQUALS(VideoClip(0, 8)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
        StartTest("Unlinked Video: Speed decrease: Verify begin frame the same");
        Scrub(LeftPixel(VideoClip(0,5)), LeftPixel(VideoClip(0,5)) + 10);
        ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5)));
        StartTest("Unlinked Video: Speed decrease: Verify end frame the same");
        Scrub(RightPixel(VideoClip(0,5)) - 10, RightPixel(VideoClip(0,5)));
        ASSERT_FRAMES_EQUAL(referenceLastFrame, LastFrame(VideoClip(0, 5)));
        Undo();
        ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1,1));
    }
    {
        StartTest("Unlinked Video: Speed increase: Reducing clip allowed, begin and end frames stay the same");
        TimelineLeftClick(Center(VideoClip(0,5)));
        ASSERT(DetailsView(VideoClip(0,5)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 10100);
        ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(2,1));
        ASSERT_EQUALS(DetailsClipView()->getSpeedSpin()->GetValue(), 2.00);
        ASSERT_EQUALS(VideoClip(0, 8)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
        StartTest("Unlinked Video: Speed increase: Verify begin frame the same");
        Scrub(LeftPixel(VideoClip(0,5)), LeftPixel(VideoClip(0,5)) + 10);
        ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5))); // Ensure first frame still the same
        StartTest("Unlinked Video: Speed increase: Verify end frame the same");
        Scrub(RightPixel(VideoClip(0,5)) - 10, RightPixel(VideoClip(0,5)));
        // NOT: ASSERT_FRAMES_EQUAL(referenceLastFrame, LastFrame(VideoClip(0, 5))); -- todo why is the last frame in the faster clip, always the last - 2 of the original clip?
        Undo();
        ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1,1));
    }
    Undo(4);
    /////////////////////////////////////////////////////////////////////////////////////////////////
    Unlink(AudioClip(0, 4));
    TimelineDeleteClip(VideoClip(0, 4));
    TimelineTrimRight(AudioClip(0, 4), -50, false);
    TimelineTrimLeft(AudioClip(0, 4), 50, false); // Note: the trim adds an empty clip, thus the clip must now be indexed with '5'

    ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 1));
    model::AudioChunkPtr referenceFirstChunk{ FirstChunk(AudioClip(0, 5)) };
    model::AudioChunkPtr referenceLastChunk{ LastChunk(AudioClip(0, 5)) };
    {
        StartTest("Unlinked Audio: Speed decrease");
        TimelineLeftClick(Center(AudioClip(0, 5)));
        ASSERT(DetailsView(AudioClip(0, 5)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
        ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 100));
        ASSERT_EQUALS(DetailsClipView()->getSpeedSpin()->GetValue(), 0.01);
        ASSERT_EQUALS(DetailsClipView()->getSpeedSlider()->GetValue(), 100);
        ASSERT_EQUALS(AudioClip(0, 8)->getLeftPts(), VideoClip(0, 6)->getLeftPts()); // Verify proper shifting
        StartTest("Unlinked Audio: Speed decrease: Verify playback");
        Play(LeftPixel(AudioClip(0, 5)), 500);
        StartTest("Unlinked Audio: Speed decrease: Verify playback");
        Play(RightPixel(AudioClip(0, 5)) - 5, 1000);
        Undo();
        ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 1));
    }
    {
        StartTest("Unlinked Audio: Speed increase: Reducing clip allowed");
        TimelineLeftClick(Center(AudioClip(0, 5)));
        ASSERT(DetailsView(AudioClip(0, 5)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 10100);
        ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(2, 1));
        ASSERT_EQUALS(DetailsClipView()->getSpeedSpin()->GetValue(), 2.00);
        ASSERT_EQUALS(AudioClip(0, 8)->getLeftPts(), VideoClip(0, 6)->getLeftPts()); // Verify proper shifting
        StartTest("Unlinked Audio: Speed increase: Verify playback");
        Play(LeftPixel(AudioClip(0, 5)), 500);
        StartTest("Unlinked Audio: Speed increase: Verify playback");
        Play(RightPixel(AudioClip(0, 5)) - 5, 1000);
        Undo();
        ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 1));
    }
    Undo(4);
    /////////////////////////////////////////////////////////////////////////////////////////////////
    TimelineTrimRight(VideoClip(0, 4), -50, false);
    TimelineTrimLeft(VideoClip(0, 4), 50, false); // Note: the trim adds an empty clip, thus the clip must now be indexed with '5'

    ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
    ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 1));
    referenceFirstFrame = FirstFrame(VideoClip(0, 5));
    referenceLastFrame = LastFrame(VideoClip(0, 5));
    {
        StartTest("Linked clips: Speed decrease: Enlarging clip allowed, begin and end frames stay the same");
        TimelineLeftClick(Center(VideoClip(0, 5)));
        ASSERT(DetailsView(VideoClip(0, 5)));
        ASSERT(DetailsView(AudioClip(0, 5)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
        ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 100));
        ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 100));
        ASSERT_EQUALS(DetailsClipView()->getSpeedSpin()->GetValue(), 0.01);
        ASSERT_EQUALS(DetailsClipView()->getSpeedSlider()->GetValue(), 100);
        ASSERT_EQUALS(VideoClip(0, 8)->getLeftPts(), AudioClip(0, 8)->getLeftPts()); // Verify proper shifting
        StartTest("Linked clips: Speed decrease: Verify begin frame the same");
        Scrub(LeftPixel(VideoClip(0, 5)), LeftPixel(VideoClip(0, 5)) + 10);
        Play(LeftPixel(AudioClip(0, 5)), 500);
        ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5)));
        StartTest("Linked clips: Speed decrease: Verify end frame the same");
        Scrub(RightPixel(VideoClip(0, 5)) - 10, RightPixel(VideoClip(0, 5)));
        Play(RightPixel(AudioClip(0, 5)) - 5, 1000);
        ASSERT_FRAMES_EQUAL(referenceLastFrame, LastFrame(VideoClip(0, 5)));
        Undo();
        ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
        ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 1));
    }
    {
        StartTest("Linked clips: Speed increase: Reducing clip allowed, begin and end frames stay the same");
        TimelineLeftClick(Center(VideoClip(0, 5)));
        ASSERT(DetailsView(VideoClip(0, 5)));
        ASSERT(DetailsView(AudioClip(0, 5)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 10100);
        ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(2, 1));
        ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(2, 1));
        ASSERT_EQUALS(DetailsClipView()->getSpeedSpin()->GetValue(), 2.00);
        ASSERT_EQUALS(VideoClip(0, 8)->getLeftPts(), AudioClip(0, 8)->getLeftPts()); // Verify proper shifting
        StartTest("Linked clips: Speed increase: Verify begin frame the same");
        Scrub(LeftPixel(VideoClip(0, 5)), LeftPixel(VideoClip(0, 5)) + 10);
        Play(LeftPixel(AudioClip(0, 5)), 500);
        ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5))); // Ensure first frame still the same
        StartTest("Linked clips: Speed increase: Verify end frame the same");
        Scrub(RightPixel(VideoClip(0, 5)) - 10, RightPixel(VideoClip(0, 5)));
        Play(RightPixel(AudioClip(0, 5)) - 5, 1000);
        Undo();
        ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
        ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 1));
    }
    Undo(2);
}


void TestClipSpeed::testChangeClipSpeedWithMismatchingTransitions()
{
    StartTestSuite();
    // If only the audio or video clip has a transition then changing the speed is not allowed.
    TimelineZoomIn(2);
    {
        StartTest("Not allowed if only the video clip has an in-transition");
        MakeInTransitionAfterClip preparation(3);
        TimelineLeftClick(Center(VideoClip(0, 5)));
        ASSERT(DetailsView(VideoClip(0, 5)));
        ASSERT(!DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(!DetailsClipView()->getSpeedSpin()->IsEnabled());
    }
    {
        StartTest("Not allowed if only the audio clip has an in-transition");
        MakeInTransitionAfterClip preparation(3, true);
        TimelineLeftClick(Center(AudioClip(0, 5)));
        ASSERT(DetailsView(AudioClip(0, 5)));
        ASSERT(!DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(!DetailsClipView()->getSpeedSpin()->IsEnabled());
    }
    {
        StartTest("Not allowed if only the video clip has an out-transition");
        MakeOutTransitionAfterClip preparation(4);
        TimelineLeftClick(Center(VideoClip(0, 4)));
        ASSERT(DetailsView(VideoClip(0, 4)));
        ASSERT(!DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(!DetailsClipView()->getSpeedSpin()->IsEnabled());
    }
    {
        StartTest("Not allowed if only the audio clip has an out-transition");
        MakeOutTransitionAfterClip preparation(4, true);
        TimelineLeftClick(Center(AudioClip(0, 4)));
        ASSERT(DetailsView(AudioClip(0, 4)));
        ASSERT(!DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(!DetailsClipView()->getSpeedSpin()->IsEnabled());
    }
    {
        StartTest("Allowed if both clips have the same length in-transition");
        MakeInTransitionAfterClip preparation(3);
        MakeInTransitionAfterClip preparation2(3, true);
        TimelineLeftClick(Center(VideoClip(0, 5)));
        ASSERT(DetailsView(VideoClip(0, 5)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
    }
    {
        StartTest("Allowed if both clips have the same length out-transition");
        MakeOutTransitionAfterClip preparation(4);
        MakeOutTransitionAfterClip preparation2(4, true);
        TimelineLeftClick(Center(VideoClip(0, 4)));
        ASSERT(DetailsView(VideoClip(0, 4)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
    }
}

void TestClipSpeed::testChangeClipSpeedWithInTransition()
{
    StartTestSuite();
    TimelineZoomIn(3);
    Unlink(VideoClip(0, 4));

#ifndef __GNUC__

    /////////////////////////////////////////////////////////////////////////////////////////////////

    TimelineDeleteClip(AudioClip(0, 4));

    {
        MakeInTransitionAfterClip preparation(3); // This makes the tested clip have number 5
        ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
        model::VideoFramePtr referenceFirstFrame{ FirstFrame(VideoClip(0, 5)) };
        model::VideoFramePtr referenceLastFrame{ LastFrame(VideoClip(0, 5)) };

        // todo gdi calls in non main thread cause crash: thumbnail being generated during scrubbing....
        {
            StartTest("Video: Speed decrease: Enlarging clip allowed, begin and end frames stay the same");
            TimelineLeftClick(Center(VideoClip(0, 5)));
            ASSERT(DetailsView(VideoClip(0, 5)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
            Scrub(LeftPixel(VideoClip(0, 5)), LeftPixel(VideoClip(0, 5)) + 10);
            ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 100));
            ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
            StartTest("Video: Speed decrease: Verify begin frame the same");
            Scrub(LeftPixel(VideoClip(0, 5)), LeftPixel(VideoClip(0, 5)) + 10);
            ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5)));
            StartTest("Video: Speed decrease: Verify end frame the same");
            Scrub(RightPixel(VideoClip(0, 5)) - 10, RightPixel(VideoClip(0, 5)));
            ASSERT_FRAMES_EQUAL(referenceLastFrame, LastFrame(VideoClip(0, 5)));
            Undo();
            ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
        {
            StartTest("Video: Speed increase: Reducing clip not allowed (need enough frames for transition)");
            TimelineLeftClick(Center(VideoClip(0, 5)));
            ASSERT(DetailsView(VideoClip(0, 5)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 10101);
            ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
        {
            StartTest("Video: Speed increase: Reducing clip allowed (when there's enough room), begin and end frames stay the same");
            TimelineTrimTransitionRightClipBegin(VideoClip(0, 4), 100);
            TimelineLeftClick(Center(VideoClip(0, 5)));
            ASSERT(DetailsView(VideoClip(0, 5)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 10100);
            ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(2, 1));
            ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
            StartTest("Video: Speed increase: Verify begin frame the same");
            Scrub(LeftPixel(VideoClip(0, 5)), LeftPixel(VideoClip(0, 5)) + 10);
            // NOT: ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5))); // Not always possible when increasing speed
            StartTest("Video: Speed increase: Verify end frame the same");
            Scrub(RightPixel(VideoClip(0, 5)) - 10, RightPixel(VideoClip(0, 5)));
            // NOT: ASSERT_FRAMES_EQUAL(referenceLastFrame, LastFrame(VideoClip(0, 5))); // Not always possible when increasing speed
            Undo();
            ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
            Undo();
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
    }
    Undo(); // Delete audio clip

    /////////////////////////////////////////////////////////////////////////////////////////////////

    TimelineDeleteClip(VideoClip(0, 4));

    {
        MakeInTransitionAfterClip preparation(3, true); // This makes the tested clip have number 5
        ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 1));

        {
            StartTest("Audio: Speed decrease: Enlarging clip allowed");
            TimelineLeftClick(Center(AudioClip(0, 5)));
            ASSERT(DetailsView(AudioClip(0, 5)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
            Scrub(LeftPixel(AudioClip(0, 5)), LeftPixel(AudioClip(0, 5)) + 10);
            ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 100));
            ASSERT_EQUALS(AudioClip(0, 7)->getLeftPts(), VideoClip(0, 6)->getLeftPts()); // Verify proper shifting
            StartTest("Audio: Speed decrease: Verify playback");
            Play(LeftPixel(AudioClip(0, 5)), 500);
            StartTest("Audio: Speed decrease: Verify playback");
            Play(LeftPixel(AudioClip(0, 5)), 500);
            Play(RightPixel(AudioClip(0, 5)) - 5, 1000);
            Undo();
            ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
        {
            StartTest("Audio: Speed increase: Reducing clip not allowed (need enough frames for transition)");
            TimelineLeftClick(Center(AudioClip(0, 5)));
            ASSERT(DetailsView(AudioClip(0, 5)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 10101);
            ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
        {
            StartTest("Audio: Speed increase: Reducing clip allowed (when there's enough room), begin and end frames stay the same");
            TimelineTrimTransitionRightClipBegin(AudioClip(0, 4), 100);
            TimelineLeftClick(Center(AudioClip(0, 5)));
            ASSERT(DetailsView(AudioClip(0, 5)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 10100);
            ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(2, 1));
            ASSERT_EQUALS(AudioClip(0, 7)->getLeftPts(), VideoClip(0, 6)->getLeftPts()); // Verify proper shifting
            StartTest("Audio: Speed increase: Verify begin frame the same");
            Play(LeftPixel(AudioClip(0, 5)), 500);
            StartTest("Audio: Speed increase: Verify end frame the same");
            Play(RightPixel(AudioClip(0, 5)) - 5, 1000);
            Undo();
            ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 1));
            Undo();
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
    }
    Undo(); // Delete audio clip

    /////////////////////////////////////////////////////////////////////////////////////////////////

    Undo(); // Unlink

    {
        MakeInTransitionAfterClip preparation(3); // This makes the tested clip have number 5
        MakeInTransitionAfterClip preparation2(3, true); // This makes the tested clip have number 5
        ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
        ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 1));
        model::VideoFramePtr referenceFirstFrame{ FirstFrame(VideoClip(0, 5)) };
        model::VideoFramePtr referenceLastFrame{ LastFrame(VideoClip(0, 5)) };

        {
            StartTest("Linked: Speed decrease: Enlarging clip allowed, begin and end frames stay the same");
            TimelineLeftClick(Center(VideoClip(0, 5)));
            ASSERT(DetailsView(VideoClip(0, 5)));
            ASSERT(DetailsView(AudioClip(0, 5)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
            Scrub(LeftPixel(VideoClip(0, 5)), LeftPixel(VideoClip(0, 5)) + 10);
            Play(LeftPixel(AudioClip(0, 5)), 500);
            ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 100));
            ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 100));
            ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 7)->getLeftPts()); // Verify proper shifting
            StartTest("Linked: Speed decrease: Verify begin frame the same");
            Scrub(LeftPixel(VideoClip(0, 5)), LeftPixel(VideoClip(0, 5)) + 10);
            ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5)));
            Play(LeftPixel(AudioClip(0, 5)), 500);
            StartTest("Linked: Speed decrease: Verify end frame the same");
            Scrub(RightPixel(VideoClip(0, 5)) - 10, RightPixel(VideoClip(0, 5)));
            ASSERT_FRAMES_EQUAL(referenceLastFrame, LastFrame(VideoClip(0, 5)));
            Play(RightPixel(AudioClip(0, 5)) - 5, 1000);
            Undo();
            ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
            ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
        {
            StartTest("Linked: Speed increase: Reducing clip not allowed (need enough frames for transition)");
            TimelineLeftClick(Center(VideoClip(0, 5)));
            ASSERT(DetailsView(VideoClip(0, 5)));
            ASSERT(DetailsView(AudioClip(0, 5)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 10101);
            ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
            ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
        {
            StartTest("Linked: Speed increase: Reducing clip allowed (when there's enough room), begin and end frames stay the same");
            TimelineTrimTransitionRightClipBegin(VideoClip(0, 4), 100);
            TimelineLeftClick(Center(VideoClip(0, 5)));
            ASSERT(DetailsView(VideoClip(0, 5)));
            ASSERT(DetailsView(AudioClip(0, 5)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 10100);
            ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(2, 1));
            ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(2, 1));
            ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 7)->getLeftPts()); // Verify proper shifting
            StartTest("Linked: Speed increase: Verify begin frame the same");
            Scrub(LeftPixel(VideoClip(0, 5)), LeftPixel(VideoClip(0, 5)) + 10);
            // NOT: ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5))); // Not always possible when increasing speed
            Play(LeftPixel(AudioClip(0, 5)), 500);
            StartTest("Video: Speed increase: Verify end frame the same");
            Scrub(RightPixel(VideoClip(0, 5)) - 10, RightPixel(VideoClip(0, 5)));
            // NOT: ASSERT_FRAMES_EQUAL(referenceLastFrame, LastFrame(VideoClip(0, 5))); // Not always possible when increasing speed
            Play(RightPixel(AudioClip(0, 5)) - 5, 1000);
            Undo();
            ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
            ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 1));
            Undo();
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
    }
#endif
}

void TestClipSpeed::testChangeClipSpeedWithInOutTransitionBefore()
{
    StartTestSuite();
    TimelineZoomIn(3);
    Unlink(VideoClip(0, 4));

    /////////////////////////////////////////////////////////////////////////////////////////////////

    TimelineDeleteClip(AudioClip(0, 4));

#ifndef __GNUC__
    {
        MakeInOutTransitionAfterClip preparation(3); // This makes the tested clip have number 5
        ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
        model::VideoFramePtr referenceFirstFrame{ FirstFrame(VideoClip(0, 5)) };
        model::VideoFramePtr referenceLastFrame{ LastFrame(VideoClip(0, 5)) };

        {
            StartTest("Video: Speed decrease: Enlarging clip allowed, begin and end frames stay the same");
            TimelineLeftClick(Center(VideoClip(0, 5)));
            ASSERT(DetailsView(VideoClip(0, 5)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
            ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 100));
            ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
            StartTest("Video: Speed decrease: Verify begin frame the same");
            Scrub(LeftPixel(VideoClip(0, 5)), LeftPixel(VideoClip(0, 5)) + 10);
            ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5)));
            StartTest("Video: Speed decrease: Verify end frame the same");
            Scrub(RightPixel(VideoClip(0, 5)) - 10, RightPixel(VideoClip(0, 5)));
            ASSERT_FRAMES_EQUAL(referenceLastFrame, LastFrame(VideoClip(0, 5)));
            Undo();
            ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
        {
            StartTest("Video: Speed increase: Reducing clip not allowed (need enough frames for transition)");
            TimelineTrimTransitionRightClipBegin(VideoClip(0, 4), -200); // Ensure that the entire 'clip offset' is required by the transition (extend as much as possible)
            TimelineLeftClick(Center(VideoClip(0, 5)));
            ASSERT(DetailsView(VideoClip(0, 5)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 10101);
            ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
            Undo();
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
        {
            StartTest("Video: Speed increase: Reducing clip allowed (when there's enough room), begin and end frames stay the same");
            // NOT: TimelineTrimTransitionRightClipBegin(VideoClip(0,4), 100); -- for creating the in-out-transition already a bit 'extra' is trimmed away.
            TimelineLeftClick(Center(VideoClip(0, 5)));
            ASSERT(DetailsView(VideoClip(0, 5)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 10100);
            ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(2, 1));
            ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
            StartTest("Video: Speed increase: Verify begin frame the same");
            Scrub(LeftPixel(VideoClip(0, 5)), LeftPixel(VideoClip(0, 5)) + 10);
            // NOT: ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5))); // Not always possible when increasing speed
            StartTest("Video: Speed increase: Verify end frame the same");
            Scrub(RightPixel(VideoClip(0, 5)) - 10, RightPixel(VideoClip(0, 5)));
            // NOT: ASSERT_FRAMES_EQUAL(referenceLastFrame, LastFrame(VideoClip(0, 5))); // Not always possible when increasing speed
            Undo();
            ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
    }

    Undo(); // Delete audio clip

    /////////////////////////////////////////////////////////////////////////////////////////////////

    TimelineDeleteClip(VideoClip(0, 4));

    {
        MakeInOutTransitionAfterClip preparation(3, true); // This makes the tested clip have number 5
        ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 1));

        {
            StartTest("Audio: Speed decrease: Enlarging clip allowed");
            TimelineLeftClick(Center(AudioClip(0, 5)));
            ASSERT(DetailsView(AudioClip(0, 5)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
            ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 100));
            ASSERT_EQUALS(AudioClip(0, 7)->getLeftPts(), VideoClip(0, 6)->getLeftPts()); // Verify proper shifting
            StartTest("Audio: Speed decrease: Verify playback");
            Play(LeftPixel(AudioClip(0, 5)), 500);
            StartTest("Audio: Speed decrease: Verify playback");
            Play(RightPixel(AudioClip(0, 5)) - 5, 1000);
            Undo();
            ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
        {
            StartTest("Audio: Speed increase: Reducing clip not allowed (need enough frames for transition)");
            TimelineTrimTransitionRightClipBegin(AudioClip(0, 4), -200); // Ensure that the entire 'clip offset' is required by the transition (extend as much as possible)
            TimelineLeftClick(Center(AudioClip(0, 5)));
            ASSERT(DetailsView(AudioClip(0, 5)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 10101);
            ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 1));
            Undo();
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
        {
            StartTest("Audio: Speed increase: Reducing clip allowed (when there's enough room)");
            // NOT: TimelineTrimTransitionRightClipBegin(VideoClip(0,4), 100); -- for creating the in-out-transition already a bit 'extra' is trimmed away.
            TimelineLeftClick(Center(AudioClip(0, 5)));
            ASSERT(DetailsView(AudioClip(0, 5)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 10100);
            ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(2, 1));
            ASSERT_EQUALS(AudioClip(0, 7)->getLeftPts(), VideoClip(0, 6)->getLeftPts()); // Verify proper shifting
            StartTest("Audio: Speed increase: Verify playback");
            Play(LeftPixel(AudioClip(0, 5)), 500);
            StartTest("Audio: Speed increase: Verify playback");
            Play(RightPixel(AudioClip(0, 5)) - 5, 1000);
            Undo();
            ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
    }

    Undo(); // Delete audio clip

    /////////////////////////////////////////////////////////////////////////////////////////////////

    Undo(); // Unlink

    {
        MakeInOutTransitionAfterClip preparation(3); // This makes the tested clip have number 5
        MakeInOutTransitionAfterClip preparation2(3, true); // This makes the tested clip have number 5
        ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
        ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 1));
        model::VideoFramePtr referenceFirstFrame{ FirstFrame(VideoClip(0, 5)) };
        model::VideoFramePtr referenceLastFrame{ LastFrame(VideoClip(0, 5)) };

        {
            StartTest("Linked: Speed decrease: Enlarging clip allowed, begin and end frames stay the same");
            TimelineLeftClick(Center(VideoClip(0, 5)));
            ASSERT(DetailsView(VideoClip(0, 5)));
            ASSERT(DetailsView(AudioClip(0, 5)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
            ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 100));
            ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 100));
            ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 7)->getLeftPts()); // Verify proper shifting
            StartTest("Linked: Speed decrease: Verify begin frame the same");
            Scrub(LeftPixel(VideoClip(0, 5)), LeftPixel(VideoClip(0, 5)) + 10);
            ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5)));
            Play(LeftPixel(AudioClip(0, 5)), 500);
            StartTest("Linked: Speed decrease: Verify end frame the same");
            Scrub(RightPixel(VideoClip(0, 5)) - 10, RightPixel(VideoClip(0, 5)));
            ASSERT_FRAMES_EQUAL(referenceLastFrame, LastFrame(VideoClip(0, 5)));
            Play(RightPixel(AudioClip(0, 5)) - 5, 1000);
            Undo();
            ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
            ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
        {
            StartTest("Linked: Speed increase: Reducing clip not allowed (need enough frames for transition)");
            TimelineTrimTransitionRightClipBegin(VideoClip(0, 4), -200); // Ensure that the entire 'clip offset' is required by the transition (extend as much as possible)
            TimelineLeftClick(Center(VideoClip(0, 5)));
            ASSERT(DetailsView(VideoClip(0, 5)));
            ASSERT(DetailsView(AudioClip(0, 5)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 10101);
            ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
            ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 1));
            Undo();
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
        {
            StartTest("Linked: Speed increase: Reducing clip allowed (when there's enough room), begin and end frames stay the same");
            // NOT: TimelineTrimTransitionRightClipBegin(VideoClip(0,4), 100); -- for creating the in-out-transition already a bit 'extra' is trimmed away.
            TimelineLeftClick(Center(VideoClip(0, 5)));
            ASSERT(DetailsView(VideoClip(0, 5)));
            ASSERT(DetailsView(AudioClip(0, 5)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 10100);
            ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(2, 1));
            ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(2, 1));
            ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 7)->getLeftPts()); // Verify proper shifting
            StartTest("Linked: Speed increase: Verify begin frame the same");
            Scrub(LeftPixel(VideoClip(0, 5)), LeftPixel(VideoClip(0, 5)) + 10);
            // NOT: ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5))); // Not always possible when increasing speed
            Play(LeftPixel(AudioClip(0, 5)), 500);
            StartTest("Linked: Speed increase: Verify end frame the same");
            Scrub(RightPixel(VideoClip(0, 5)) - 10, RightPixel(VideoClip(0, 5)));
            // NOT: ASSERT_FRAMES_EQUAL(referenceLastFrame, LastFrame(VideoClip(0, 5))); // Not always possible when increasing speed
            Play(RightPixel(AudioClip(0, 5)) - 5, 1000);
            Undo();
            ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
            ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
    }

#endif
}

void TestClipSpeed::testChangeClipSpeedWithOutTransition()
{
    StartTestSuite();
    TimelineZoomIn(3);
    Unlink(VideoClip(0, 4));

    /////////////////////////////////////////////////////////////////////////////////////////////////

    TimelineDeleteClip(AudioClip(0, 4));

#ifndef __GNUC__
    {
        MakeOutTransitionAfterClip preparation(4);
        ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 1));
        model::VideoFramePtr referenceFirstFrame{ FirstFrame(VideoClip(0, 4)) };
        model::VideoFramePtr referenceLastFrame{ LastFrame(VideoClip(0, 4)) };

        {
            StartTest("Video: Speed decrease: Enlarging clip allowed, begin and end frames stay the same");
            TimelineLeftClick(Center(VideoClip(0, 4)));
            ASSERT(DetailsView(VideoClip(0, 4)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
            ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 100));
            ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
            StartTest("Video: Speed decrease: Verify begin frame the same");
            Scrub(LeftPixel(VideoClip(0, 4)), LeftPixel(VideoClip(0, 4)) + 10);
            ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 4)));
            StartTest("Video: Speed decrease: Verify end frame the same");
            Scrub(RightPixel(VideoClip(0, 4)) - 10, RightPixel(VideoClip(0, 4)));
            ASSERT_FRAMES_EQUAL(referenceLastFrame, LastFrame(VideoClip(0, 4)));
            Undo();
            ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
        {
            StartTest("Video: Speed increase: Reducing clip not allowed (need enough frames for transition)");
            TimelineLeftClick(Center(VideoClip(0, 4)));
            ASSERT(DetailsView(VideoClip(0, 4)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 10101);
            ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
        {
            StartTest("Video: Speed increase: Reducing clip allowed (when there's enough room), begin and end frames stay the same");
            TimelineTrimTransitionLeftClipEnd(VideoClip(0, 5), -100);
            TimelineLeftClick(Center(VideoClip(0, 4)));
            ASSERT(DetailsView(VideoClip(0, 4)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 10100);
            ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(2, 1));
            ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
            StartTest("Video: Speed increase: Verify begin frame the same");
            Scrub(LeftPixel(VideoClip(0, 4)), LeftPixel(VideoClip(0, 4)) + 10);
            // NOT: ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5))); // Not always possible when increasing speed
            StartTest("Video: Speed increase: Verify end frame the same");
            Scrub(RightPixel(VideoClip(0, 4)) - 10, RightPixel(VideoClip(0, 4)));
            // NOT: ASSERT_FRAMES_EQUAL(referenceLastFrame, LastFrame(VideoClip(0, 5))); // Not always possible when increasing speed
            Undo();
            ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 1));
            Undo();
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
    }

    Undo(); // Delete audio clip

    /////////////////////////////////////////////////////////////////////////////////////////////////

    TimelineDeleteClip(VideoClip(0, 4));

    {
        MakeOutTransitionAfterClip preparation(4, true);
        ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 1));

        {
            StartTest("Audio: Speed decrease: Enlarging clip allowed");
            TimelineLeftClick(Center(AudioClip(0, 4)));
            ASSERT(DetailsView(AudioClip(0, 4)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
            ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 100));
            ASSERT_EQUALS(AudioClip(0, 7)->getLeftPts(), VideoClip(0, 6)->getLeftPts()); // Verify proper shifting
            StartTest("Audio: Speed decrease: Verify playback");
            Play(LeftPixel(AudioClip(0, 4)), 500);
            StartTest("Audio: Speed decrease: Verify playback");
            Play(RightPixel(AudioClip(0, 4)) - 5, 1000);
            Undo();
            ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
        {
            StartTest("Audio: Speed increase: Reducing clip not allowed (need enough frames for transition)");
            TimelineLeftClick(Center(AudioClip(0, 4)));
            ASSERT(DetailsView(AudioClip(0, 4)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 10101);
            ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
        {
            StartTest("Audio: Speed increase: Reducing clip allowed (when there's enough room), begin and end frames stay the same");
            TimelineTrimTransitionLeftClipEnd(AudioClip(0, 5), -100);
            TimelineLeftClick(Center(AudioClip(0, 4)));
            ASSERT(DetailsView(AudioClip(0, 4)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 10100);
            ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(2, 1));
            ASSERT_EQUALS(AudioClip(0, 7)->getLeftPts(), VideoClip(0, 6)->getLeftPts()); // Verify proper shifting
            StartTest("Audio: Speed increase: Verify playback");
            Play(LeftPixel(AudioClip(0, 4)), 500);
            StartTest("Audio: Speed increase: Verify playback");
            Play(RightPixel(AudioClip(0, 4)) - 5, 1000);
            Undo();
            ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 1));
            Undo();
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
    }

    Undo(); // Delete video clip

    /////////////////////////////////////////////////////////////////////////////////////////////////

    Undo(); // Unlink

    {
        MakeOutTransitionAfterClip preparation(4);
        MakeOutTransitionAfterClip preparation2(4, true);
        ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 1));
        ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 1));
        model::VideoFramePtr referenceFirstFrame{ FirstFrame(VideoClip(0, 4)) };
        model::VideoFramePtr referenceLastFrame{ LastFrame(VideoClip(0, 4)) };

        {
            StartTest("Linked: Speed decrease: Enlarging clip allowed, begin and end frames stay the same");
            TimelineLeftClick(Center(VideoClip(0, 4)));
            ASSERT(DetailsView(VideoClip(0, 4)));
            ASSERT(DetailsView(AudioClip(0, 4)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
            ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 100));
            ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 100));
            ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 7)->getLeftPts()); // Verify proper shifting
            StartTest("Linked: Speed decrease: Verify begin frame the same");
            Scrub(LeftPixel(VideoClip(0, 4)), LeftPixel(VideoClip(0, 4)) + 10);
            ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 4)));
            Play(LeftPixel(AudioClip(0, 4)), 500);
            StartTest("Linked: Speed decrease: Verify end frame the same");
            Scrub(RightPixel(VideoClip(0, 4)) - 10, RightPixel(VideoClip(0, 4)));
            ASSERT_FRAMES_EQUAL(referenceLastFrame, LastFrame(VideoClip(0, 4)));
            Play(RightPixel(AudioClip(0, 4)) - 5, 1000);
            Undo();
            ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 1));
            ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
        {
            StartTest("Linked: Speed increase: Reducing clip not allowed (need enough frames for transition)");
            TimelineLeftClick(Center(VideoClip(0, 4)));
            ASSERT(DetailsView(VideoClip(0, 4)));
            ASSERT(DetailsView(AudioClip(0, 4)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 10101);
            ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 1));
            ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
        {
            StartTest("Linked: Speed increase: Reducing clip allowed (when there's enough room), begin and end frames stay the same");
            TimelineTrimTransitionLeftClipEnd(VideoClip(0, 5), -100);
            TimelineLeftClick(Center(VideoClip(0, 4)));
            ASSERT(DetailsView(VideoClip(0, 4)));
            ASSERT(DetailsView(AudioClip(0, 4)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 10100);
            ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(2, 1));
            ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(2, 1));
            ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 7)->getLeftPts()); // Verify proper shifting
            StartTest("Linked: Speed increase: Verify begin frame the same");
            Scrub(LeftPixel(VideoClip(0, 4)), LeftPixel(VideoClip(0, 4)) + 10);
            // NOT: ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5))); // Not always possible when increasing speed
            Play(LeftPixel(AudioClip(0, 4)), 500);
            StartTest("Linked: Speed increase: Verify end frame the same");
            Scrub(RightPixel(VideoClip(0, 4)) - 10, RightPixel(VideoClip(0, 4)));
            // NOT: ASSERT_FRAMES_EQUAL(referenceLastFrame, LastFrame(VideoClip(0, 5))); // Not always possible when increasing speed
            Play(RightPixel(AudioClip(0, 4)) - 5, 100);
            Undo();
            ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 1));
            ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 1));
            Undo();
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
    }
#endif
}

void TestClipSpeed::testChangeClipSpeedWithInOutTransitionAfter()
{
    StartTestSuite();
    TimelineZoomIn(3);
    Unlink(VideoClip(0, 4));

#ifndef __GNUC__

    /////////////////////////////////////////////////////////////////////////////////////////////////

    TimelineDeleteClip(AudioClip(0, 4));

    {
        MakeInOutTransitionAfterClip preparation(4);
        ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 1));
        model::VideoFramePtr referenceFirstFrame{ FirstFrame(VideoClip(0, 4)) };
        model::VideoFramePtr referenceLastFrame{ LastFrame(VideoClip(0, 4)) };

        {
            StartTest("Video: Speed decrease: Enlarging clip allowed, begin and end frames stay the same");
            TimelineLeftClick(Center(VideoClip(0, 4)));
            ASSERT(DetailsView(VideoClip(0, 4)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
            ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 100));
            ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
            StartTest("Video: Speed decrease: Verify begin frame the same");
            Scrub(LeftPixel(VideoClip(0, 4)), LeftPixel(VideoClip(0, 4)) + 10);
            ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 4)));
            StartTest("Video: Speed decrease: Verify end frame the same");
            Scrub(RightPixel(VideoClip(0, 4)) - 10, RightPixel(VideoClip(0, 4)));
            ASSERT_FRAMES_EQUAL(referenceLastFrame, LastFrame(VideoClip(0, 4)));
            Undo();
            ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
        {
            StartTest("Video: Speed increase: Reducing clip not allowed (need enough frames for transition)");
            TimelineTrimTransitionLeftClipEnd(VideoClip(0, 5), +200); // Ensure that the entire 'clip offset' is required by the transition (extend as much as possible)
            TimelineLeftClick(Center(VideoClip(0, 4)));
            ASSERT(DetailsView(VideoClip(0, 4)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 10101);
            ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 1));
            Undo();
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
        {
            StartTest("Video: Speed increase: Reducing clip allowed (when there's enough room), begin and end frames stay the same");
            // NOT: TimelineTrimTransitionLeftClipEnd(VideoClip(0,5), -100);  -- for creating the in-out-transition already a bit 'extra' is trimmed away.
            TimelineLeftClick(Center(VideoClip(0, 4)));
            ASSERT(DetailsView(VideoClip(0, 4)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 10100);
            ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(2, 1));
            ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
            StartTest("Video: Speed increase: Verify begin frame the same");
            Scrub(LeftPixel(VideoClip(0, 4)), LeftPixel(VideoClip(0, 4)) + 10);
            // NOT: ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5))); // Not always possible when increasing speed
            StartTest("Video: Speed increase: Verify end frame the same");
            Scrub(RightPixel(VideoClip(0, 4)) - 10, RightPixel(VideoClip(0, 4)));
            // NOT: ASSERT_FRAMES_EQUAL(referenceLastFrame, LastFrame(VideoClip(0, 5))); // Not always possible when increasing speed
            Undo();
            ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
    }

    Undo(); // Delete audio clip

    /////////////////////////////////////////////////////////////////////////////////////////////////

    TimelineDeleteClip(VideoClip(0, 4));

    {
        MakeInOutTransitionAfterClip preparation(4, true);
        ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 1));

        {
            StartTest("Audio: Speed decrease: Enlarging clip allowed, begin and end frames stay the same");
            TimelineLeftClick(Center(AudioClip(0, 4)));
            ASSERT(DetailsView(AudioClip(0, 4)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
            ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 100));
            ASSERT_EQUALS(AudioClip(0, 7)->getLeftPts(), VideoClip(0, 6)->getLeftPts()); // Verify proper shifting
            StartTest("Audio: Speed decrease: Verify playback");
            Play(LeftPixel(AudioClip(0, 4)), 500);
            StartTest("Audio: Speed decrease: Verify playback");
            Play(RightPixel(AudioClip(0, 4)) - 5, 1000);
            Undo();
            ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
        {
            StartTest("Audio: Speed increase: Reducing clip not allowed (need enough frames for transition)");
            TimelineTrimTransitionLeftClipEnd(AudioClip(0, 5), +200); // Ensure that the entire 'clip offset' is required by the transition (extend as much as possible)
            TimelineLeftClick(Center(AudioClip(0, 4)));
            ASSERT(DetailsView(AudioClip(0, 4)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 10101);
            ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 1));
            Undo();
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
        {
            StartTest("Audio: Speed increase: Reducing clip allowed (when there's enough room), begin and end frames stay the same");
            // NOT: TimelineTrimTransitionLeftClipEnd(AudioClip(0,5), -100);  -- for creating the in-out-transition already a bit 'extra' is trimmed away.
            TimelineLeftClick(Center(AudioClip(0, 4)));
            ASSERT(DetailsView(AudioClip(0, 4)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 10100);
            ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(2, 1));
            ASSERT_EQUALS(AudioClip(0, 7)->getLeftPts(), VideoClip(0, 6)->getLeftPts()); // Verify proper shifting
            StartTest("Audio: Speed increase: Verify playback");
            Play(LeftPixel(AudioClip(0, 4)), 500);
            StartTest("Audio: Speed increase: Verify playback");
            Play(RightPixel(AudioClip(0, 4)) - 5, 1000);
            Undo();
            ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
    }

    Undo(); // Delete video clip

    /////////////////////////////////////////////////////////////////////////////////////////////////

    Undo(); // Unlink

    {
        MakeInOutTransitionAfterClip preparation(4);
        MakeInOutTransitionAfterClip preparation2(4, true);
        ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 1));
        ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 1));
        model::VideoFramePtr referenceFirstFrame{ FirstFrame(VideoClip(0, 4)) };
        model::VideoFramePtr referenceLastFrame{ LastFrame(VideoClip(0, 4)) };

        {
            StartTest("Linked: Speed decrease: Enlarging clip allowed, begin and end frames stay the same");
            TimelineLeftClick(Center(VideoClip(0, 4)));
            ASSERT(DetailsView(VideoClip(0, 4)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
            ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 100));
            ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 100));
            ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 7)->getLeftPts()); // Verify proper shifting
            StartTest("Linked: Speed decrease: Verify begin frame the same");
            Scrub(LeftPixel(VideoClip(0, 4)), LeftPixel(VideoClip(0, 4)) + 10);
            ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 4)));
            Play(LeftPixel(AudioClip(0, 4)), 500);
            StartTest("Linked: Speed decrease: Verify end frame the same");
            Scrub(RightPixel(VideoClip(0, 4)) - 10, RightPixel(VideoClip(0, 4)));
            ASSERT_FRAMES_EQUAL(referenceLastFrame, LastFrame(VideoClip(0, 4)));
            Play(RightPixel(AudioClip(0, 4)) - 5, 1000);
            Undo();
            ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 1));
            ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
        {
            StartTest("Linked: Speed increase: Reducing clip not allowed (need enough frames for transition)");
            TimelineTrimTransitionLeftClipEnd(VideoClip(0, 5), +200); // Ensure that the entire 'clip offset' is required by the transition (extend as much as possible)
            TimelineLeftClick(Center(VideoClip(0, 4)));
            ASSERT(DetailsView(VideoClip(0, 4)));
            ASSERT(DetailsView(AudioClip(0, 4)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 10101);
            ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 1));
            ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 1));
            Undo();
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
        {
            StartTest("Linked: Speed increase: Reducing clip allowed (when there's enough room), begin and end frames stay the same");
            // NOT: TimelineTrimTransitionLeftClipEnd(VideoClip(0,5), -100);  -- for creating the in-out-transition already a bit 'extra' is trimmed away.
            TimelineLeftClick(Center(VideoClip(0, 4)));
            ASSERT(DetailsView(VideoClip(0, 4)));
            ASSERT(DetailsView(AudioClip(0, 4)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 10100);
            ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(2, 1));
            ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(2, 1));
            ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 7)->getLeftPts()); // Verify proper shifting
            StartTest("Linked: Speed increase: Verify begin frame the same");
            Scrub(LeftPixel(VideoClip(0, 4)), LeftPixel(VideoClip(0, 4)) + 10);
            // NOT: ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5))); // Not always possible when increasing speed
            Play(LeftPixel(AudioClip(0, 4)), 500);
            StartTest("Linked: Speed increase: Verify end frame the same");
            Scrub(RightPixel(VideoClip(0, 4)) - 10, RightPixel(VideoClip(0, 4)));
            // NOT: ASSERT_FRAMES_EQUAL(referenceLastFrame, LastFrame(VideoClip(0, 5))); // Not always possible when increasing speed
            Play(RightPixel(AudioClip(0, 4)) - 5, 1000);
            Undo();
            ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 1));
            ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
    }

#endif
}

void TestClipSpeed::testChangeClipSpeedWithAdjacentTransitions()
{
    StartTestSuite();
    TimelineZoomIn(3);
    Unlink(VideoClip(0, 4));

#ifndef __GNUC__

    /////////////////////////////////////////////////////////////////////////////////////////////////

    {
        TimelineDeleteClip(AudioClip(0, 4));

        ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 1));
        model::VideoFramePtr referenceFirstFrame{ FirstFrame(VideoClip(0, 4)) };
        model::VideoFramePtr referenceLastFrame{ LastFrame(VideoClip(0, 4)) };

        {
            MakeOutTransitionAfterClip preparation(3);
            {
                StartTest("Video: Speed decrease: Enlarging clip allowed, begin and end frames stay the same");
                TimelineLeftClick(Center(VideoClip(0, 5)));
                ASSERT(DetailsView(VideoClip(0, 5)));
                ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
                ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
                SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
                ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 100));
                ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
                StartTest("Video: Speed decrease: Verify begin frame the same");
                Scrub(LeftPixel(VideoClip(0, 5)), LeftPixel(VideoClip(0, 5)) + 10);
                ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5)));
                StartTest("Video: Speed decrease: Verify end frame the same");
                Scrub(RightPixel(VideoClip(0, 5)) - 10, RightPixel(VideoClip(0, 5)));
                ASSERT_FRAMES_EQUAL(referenceLastFrame, LastFrame(VideoClip(0, 5)));
                Undo();
                ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
                ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
            }
            {
                StartTest("Video: Speed increase: Reducing clip allowed (enough room), begin and end frames stay the same");
                TimelineLeftClick(Center(VideoClip(0, 5)));
                ASSERT(DetailsView(VideoClip(0, 5)));
                ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
                ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
                SetValue(DetailsClipView()->getSpeedSlider(), 10100);
                ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(2, 1));
                ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
                StartTest("Video: Speed increase: Verify begin frame the same");
                Scrub(LeftPixel(VideoClip(0, 5)), LeftPixel(VideoClip(0, 5)) + 10);
                ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5)));
                StartTest("Video: Speed increase: Verify end frame the same");
                Scrub(RightPixel(VideoClip(0, 5)) - 10, RightPixel(VideoClip(0, 5)));
                // NOT: ASSERT_FRAMES_EQUAL(referenceLastFrame, LastFrame(VideoClip(0, 5))); // Not always possible when increasing speed
                Undo();
                ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
                ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
            }
        }
        {
            MakeInTransitionAfterClip preparation(4);
            {
                StartTest("Video: Speed decrease: Enlarging clip allowed, begin and end frames stay the same");
                TimelineLeftClick(Center(VideoClip(0, 4)));
                ASSERT(DetailsView(VideoClip(0, 4)));
                ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
                ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
                SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
                ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 100));
                ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
                StartTest("Video: Speed decrease: Verify begin frame the same");
                Scrub(LeftPixel(VideoClip(0, 4)), LeftPixel(VideoClip(0, 4)) + 10);
                ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 4)));
                StartTest("Video: Speed decrease: Verify end frame the same");
                Scrub(RightPixel(VideoClip(0, 4)) - 10, RightPixel(VideoClip(0, 4)));
                ASSERT_FRAMES_EQUAL(referenceLastFrame, LastFrame(VideoClip(0, 4)));
                Undo();
                ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 1));
                ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
            }
            {
                StartTest("Video: Speed increase: Reducing clip allowed (enough room), begin and end frames stay the same");
                TimelineLeftClick(Center(VideoClip(0, 4)));
                ASSERT(DetailsView(VideoClip(0, 4)));
                ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
                ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
                SetValue(DetailsClipView()->getSpeedSlider(), 10100);
                ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(2, 1));
                ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
                StartTest("Video: Speed increase: Verify begin frame the same");
                Scrub(LeftPixel(VideoClip(0, 4)), LeftPixel(VideoClip(0, 4)) + 10);
                ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 4)));
                StartTest("Video: Speed increase: Verify end frame the same");
                Scrub(RightPixel(VideoClip(0, 4)) - 10, RightPixel(VideoClip(0, 4)));
                // NOT: ASSERT_FRAMES_EQUAL(referenceLastFrame, LastFrame(VideoClip(0, 5))); // Not always possible when increasing speed
                Undo();
                ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 1));
                ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
            }
        }

        Undo(); // Delete audio clip
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////

    {
        TimelineDeleteClip(VideoClip(0, 4));

        ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 1));

        {
            MakeOutTransitionAfterClip preparation(3, true);
            {
                StartTest("Audio: Speed decrease: Enlarging clip allowed, begin and end frames stay the same");
                TimelineLeftClick(Center(AudioClip(0, 5)));
                ASSERT(DetailsView(AudioClip(0, 5)));
                ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
                ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
                SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
                ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 100));
                ASSERT_EQUALS(AudioClip(0, 7)->getLeftPts(), VideoClip(0, 6)->getLeftPts()); // Verify proper shifting
                StartTest("Audio: Speed decrease: Verify playback");
                Play(LeftPixel(AudioClip(0, 5)), 500);
                StartTest("Audio: Speed decrease: Verify playback");
                Play(RightPixel(AudioClip(0, 5)) - 5, 1000);
                Undo();
                ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 1));
                ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
            }
            {
                StartTest("Audio: Speed increase: Reducing clip allowed (enough room), begin and end frames stay the same");
                TimelineLeftClick(Center(AudioClip(0, 5)));
                ASSERT(DetailsView(AudioClip(0, 5)));
                ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
                ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
                SetValue(DetailsClipView()->getSpeedSlider(), 10100);
                ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(2, 1));
                ASSERT_EQUALS(AudioClip(0, 7)->getLeftPts(), VideoClip(0, 6)->getLeftPts()); // Verify proper shifting
                StartTest("Audio: Speed increase: Verify playback");
                Play(LeftPixel(AudioClip(0, 5)), 500);
                StartTest("Audio: Speed increase: Verify playback");
                Play(RightPixel(AudioClip(0, 5)) - 5, 1000);
                Undo();
                ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 1));
                ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
            }
        }
        {
            MakeInTransitionAfterClip preparation(4, true);
            {
                StartTest("Audio: Speed decrease: Enlarging clip allowed, begin and end frames stay the same");
                TimelineLeftClick(Center(AudioClip(0, 4)));
                ASSERT(DetailsView(AudioClip(0, 4)));
                ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
                ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
                SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
                ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 100));
                ASSERT_EQUALS(AudioClip(0, 7)->getLeftPts(), VideoClip(0, 6)->getLeftPts()); // Verify proper shifting
                StartTest("Audio: Speed decrease: Verify playback");
                Play(LeftPixel(AudioClip(0, 4)), 500);
                StartTest("Audio: Speed decrease: Verify playback");
                Play(RightPixel(AudioClip(0, 4)) - 5, 1000);
                Undo();
                ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 1));
                ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
            }
            {
                StartTest("Audio: Speed increase: Reducing clip allowed (enough room), begin and end frames stay the same");
                TimelineLeftClick(Center(AudioClip(0, 4)));
                ASSERT(DetailsView(AudioClip(0, 4)));
                ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
                ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
                SetValue(DetailsClipView()->getSpeedSlider(), 10100);
                ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(2, 1));
                ASSERT_EQUALS(AudioClip(0, 7)->getLeftPts(), VideoClip(0, 6)->getLeftPts()); // Verify proper shifting
                StartTest("Audio: Speed increase: Verify playback");
                Play(LeftPixel(AudioClip(0, 4)), 500);
                StartTest("Audio: Speed increase: Verify playback");
                Play(RightPixel(AudioClip(0, 4)) - 5, 1000);
                Undo();
                ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 1));
                ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
            }
        }

        Undo(); // Delete video clip
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////

    {
        Undo(); // Unlink

        ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 1));
        ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 1));
        model::VideoFramePtr referenceFirstFrame{ FirstFrame(VideoClip(0, 4)) };
        model::VideoFramePtr referenceLastFrame{ LastFrame(VideoClip(0, 4)) };

        {
            MakeOutTransitionAfterClip preparation(3);
            MakeOutTransitionAfterClip preparation2(3, true);
            {
                StartTest("Linked: Speed decrease: Enlarging clip allowed, begin and end frames stay the same");
                TimelineLeftClick(Center(VideoClip(0, 5)));
                ASSERT(DetailsView(VideoClip(0, 5)));
                ASSERT(DetailsView(AudioClip(0, 5)));
                ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
                ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
                SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
                ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 100));
                ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 100));
                ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 7)->getLeftPts()); // Verify proper shifting
                StartTest("Linked: Speed decrease: Verify begin frame the same");
                Scrub(LeftPixel(VideoClip(0, 5)), LeftPixel(VideoClip(0, 5)) + 10);
                ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5)));
                Play(LeftPixel(AudioClip(0, 5)), 500);
                StartTest("Linked: Speed decrease: Verify end frame the same");
                Scrub(RightPixel(VideoClip(0, 5)) - 10, RightPixel(VideoClip(0, 5)));
                ASSERT_FRAMES_EQUAL(referenceLastFrame, LastFrame(VideoClip(0, 5)));
                Play(RightPixel(AudioClip(0, 5)) - 5, 1000);
                Undo();
                ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
                ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 1));
                ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
            }
            {
                StartTest("Linked: Speed increase: Reducing clip allowed (enough room), begin and end frames stay the same");
                TimelineLeftClick(Center(VideoClip(0, 5)));
                ASSERT(DetailsView(VideoClip(0, 5)));
                ASSERT(DetailsView(AudioClip(0, 5)));
                ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
                ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
                SetValue(DetailsClipView()->getSpeedSlider(), 10100);
                ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(2, 1));
                ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(2, 1));
                ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 7)->getLeftPts()); // Verify proper shifting
                StartTest("Linked: Speed increase: Verify begin frame the same");
                Scrub(LeftPixel(VideoClip(0, 5)), LeftPixel(VideoClip(0, 5)) + 10);
                ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5)));
                Play(LeftPixel(AudioClip(0, 5)), 500);
                StartTest("Linked: Speed increase: Verify end frame the same");
                Scrub(RightPixel(VideoClip(0, 5)) - 10, RightPixel(VideoClip(0, 5)));
                // NOT: ASSERT_FRAMES_EQUAL(referenceLastFrame, LastFrame(VideoClip(0, 5))); // Not always possible when increasing speed
                Play(RightPixel(AudioClip(0, 5)) - 5, 1000);
                Undo();
                ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
                ASSERT_CLIP_SPEED(AudioClip(0, 5), rational64(1, 1));
                ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
            }
        }
        {
            MakeInTransitionAfterClip preparation(4);
            MakeInTransitionAfterClip preparation2(4, true);
            {
                StartTest("Linked: Speed decrease: Enlarging clip allowed, begin and end frames stay the same");
                TimelineLeftClick(Center(VideoClip(0, 4)));
                ASSERT(DetailsView(VideoClip(0, 4)));
                ASSERT(DetailsView(AudioClip(0, 4)));
                ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
                ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
                SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
                ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 100));
                ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 100));
                ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 7)->getLeftPts()); // Verify proper shifting
                StartTest("Linked: Speed decrease: Verify begin frame the same");
                Scrub(LeftPixel(VideoClip(0, 4)), LeftPixel(VideoClip(0, 4)) + 10);
                ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 4)));
                Play(LeftPixel(AudioClip(0, 4)), 500);
                StartTest("Linked: Speed decrease: Verify end frame the same");
                Scrub(RightPixel(VideoClip(0, 4)) - 10, RightPixel(VideoClip(0, 4)));
                ASSERT_FRAMES_EQUAL(referenceLastFrame, LastFrame(VideoClip(0, 4)));
                Play(RightPixel(AudioClip(0, 4)) - 5, 1000);
                Undo();
                ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 1));
                ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 1));
                ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
            }
            {
                StartTest("Linked: Speed increase: Reducing clip allowed (enough room), begin and end frames stay the same");
                TimelineLeftClick(Center(VideoClip(0, 4)));
                ASSERT(DetailsView(VideoClip(0, 4)));
                ASSERT(DetailsView(AudioClip(0, 4)));
                ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
                ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
                SetValue(DetailsClipView()->getSpeedSlider(), 10100);
                ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(2, 1));
                ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(2, 1));
                ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 7)->getLeftPts()); // Verify proper shifting
                StartTest("Linked: Speed increase: Verify begin frame the same");
                Scrub(LeftPixel(VideoClip(0, 4)), LeftPixel(VideoClip(0, 4)) + 10);
                ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 4)));
                Play(LeftPixel(AudioClip(0, 4)), 500);
                StartTest("Linked: Speed increase: Verify end frame the same");
                Scrub(RightPixel(VideoClip(0, 4)) - 10, RightPixel(VideoClip(0, 4)));
                // NOT: ASSERT_FRAMES_EQUAL(referenceLastFrame, LastFrame(VideoClip(0, 5))); // Not always possible when increasing speed
                Play(RightPixel(AudioClip(0, 4)) - 5, 1000);
                Undo();
                ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 1));
                ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 1));
                ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
            }
        }
    }
#endif
}

} // namespace
