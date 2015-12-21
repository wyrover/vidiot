// Copyright 2015 Eric Raijmakers.
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
        StartTest("Not allowed for linked clips");
        TimelineLeftClick(Center(VideoClip(0,4)));
        ASSERT(DetailsView(VideoClip(0,4)));
        ASSERT(!DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(!DetailsClipView()->getSpeedSpin()->IsEnabled());
    }
    Unlink(VideoClip(0, 4));
    TimelineSelectClips({});
    {
        StartTest("Allowed for Unlinked video");
        TimelineLeftClick(Center(VideoClip(0,4)));
        ASSERT(DetailsView(VideoClip(0,4)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
    }
    {
        StartTest("Not allowed for unlinked audio");
        TimelineLeftClick(Center(AudioClip(0,4)));
        ASSERT(DetailsView(AudioClip(0,4)));
        ASSERT(!DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(!DetailsClipView()->getSpeedSpin()->IsEnabled());
    }
    {
        StartTest("Decrease (size enlargement) not allowed when clip in other track");
        TimelineLeftClick(Center(VideoClip(0,4)));
        ASSERT(DetailsView(VideoClip(0,4)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 5000); // 5000 'to the left' sets speed to 0.5
        ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1,1));
        ASSERT_EQUALS(DetailsClipView()->getSpeedSpin()->GetValue(), 1.0);
    }
    {
        StartTest("Increase (size reduction) not allowed when clip in other track");
        TimelineLeftClick(Center(VideoClip(0,4)));
        ASSERT(DetailsView(VideoClip(0,4)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 15000); // 5000 'to the right' sets speed to 50.5
        ASSERT_CLIP_SPEED(VideoClip(0,4), 1);
        ASSERT_EQUALS(DetailsClipView()->getSpeedSpin()->GetValue(), 1.0);
    }
    {
        StartTest("Increase (size reduction) not allowed when clip in other track (partial on begin)");
        TimelineSelectClips({});
        TimelineDrag(From(Center(AudioClip(0, 4))).To(LeftCenter(AudioClip(0,4))));
        TimelineLeftClick(Center(VideoClip(0,4)));
        ASSERT(DetailsView(VideoClip(0,4)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 15000);
        ASSERT_CLIP_SPEED(VideoClip(0,4), 1);
        ASSERT_EQUALS(DetailsClipView()->getSpeedSpin()->GetValue(), 1.0);
        Undo(1);
    }
    {
        StartTest("Increase (size reduction) not allowed when clip in other track (partial on end)");
        TimelineSelectClips({});
        TimelineDrag(From(Center(AudioClip(0, 4))).To(RightCenter(AudioClip(0,4))));
        TimelineLeftClick(Center(VideoClip(0,4)));
        ASSERT(DetailsView(VideoClip(0,4)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 15000);
        ASSERT_CLIP_SPEED(VideoClip(0,4), 1);
        ASSERT_EQUALS(DetailsClipView()->getSpeedSpin()->GetValue(), 1.0);
        Undo(1);
    }
    {
        StartTest("Increase (size reduction) allowed when empty clips in other tracks");
        TimelineDeleteClip(AudioClip(0,4));
        TimelineLeftClick(Center(VideoClip(0,4)));
        ASSERT(DetailsView(VideoClip(0,4)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 14900);
        ASSERT_CLIP_SPEED(VideoClip(0,4), rational64(50,1));
        ASSERT_EQUALS(DetailsClipView()->getSpeedSpin()->GetValue(), 50.0);
        ASSERT_EQUALS(VideoClip(0, 6)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
        Undo(2);
    }
    {
        StartTest("Increase (size reduction) allowed when no clips in other tracks");
        TimelineDeleteClips({ AudioClip(0, 0), AudioClip(0, 1), AudioClip(0, 2), AudioClip(0, 3), AudioClip(0, 4), AudioClip(0, 5), AudioClip(0, 6) });
        ASSERT_AUDIOTRACK0SIZE(0);
        TimelineLeftClick(Center(VideoClip(0,1)));
        ASSERT(DetailsView(VideoClip(0,1)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 14900);
        ASSERT_EQUALS(DetailsClipView()->getSpeedSpin()->GetValue(), 50.0);
        ASSERT_EQUALS(DetailsClipView()->getSpeedSlider()->GetValue(), 14900);
        ASSERT_CLIP_SPEED(VideoClip(0,1), rational64(50,1));
        Undo(2);
    }
    {
        StartTest("Increase (size reduction) not allowed with empty clips in other tracks, with enough length, but shifted.");
        Unlink(VideoClip(0, 3));
        Unlink(VideoClip(0, 5));
        TimelineDeleteClip(AudioClip(0, 4));
        TimelineDrag(From(Center(AudioClip(0, 3))).MoveLeft(20));
        TimelineDrag(From(Center(AudioClip(0, 5))).MoveLeft(20)); // Now the empty clip 'under' the to-be-changed clip has a large enough length, but does not fully cover the clip timeframe.
        TimelineLeftClick(Center(VideoClip(0, 4)));
        ASSERT(DetailsView(VideoClip(0, 4)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 15000);
        ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 1));
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
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 5000); // 5000 'to the left' sets speed to 0.5
        ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 1));
        ASSERT_EQUALS(DetailsClipView()->getSpeedSpin()->GetValue(), 1.0);
        Undo(5);
        ASSERT_EQUALS(VideoClip(0, 6)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
    }
    {
        StartTest("Change spin: Decrease (size enlargement)");
        TimelineDeleteClips({ AudioClip(0, 0), AudioClip(0, 1), AudioClip(0, 2), AudioClip(0, 3), AudioClip(0, 4), AudioClip(0, 5), AudioClip(0, 6) });
        ASSERT_AUDIOTRACK0SIZE(0);
        TimelineLeftClick(Center(VideoClip(0,1)));
        ASSERT(DetailsView(VideoClip(0,1)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSpin(), 0.5);
        ASSERT_CLIP_SPEED(VideoClip(0,1), rational64(1,2));
        ASSERT_EQUALS(DetailsClipView()->getSpeedSlider()->GetValue(), 5000);
        ASSERT_EQUALS(DetailsClipView()->getSpeedSpin()->GetValue(), 0.5);
        Undo(2);
    }
{
        StartTest("Change spin: Increase (size reduction)");
        TimelineDeleteClips({ AudioClip(0, 0), AudioClip(0, 1), AudioClip(0, 2), AudioClip(0, 3), AudioClip(0, 4), AudioClip(0, 5), AudioClip(0, 6) });
        ASSERT_AUDIOTRACK0SIZE(0);
        TimelineLeftClick(Center(VideoClip(0,1)));
        ASSERT(DetailsView(VideoClip(0,1)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSpin(), 50.0);
        ASSERT_CLIP_SPEED(VideoClip(0,1), rational64(50,1));
        ASSERT_EQUALS(DetailsClipView()->getSpeedSlider()->GetValue(), 14900);
        ASSERT_EQUALS(DetailsClipView()->getSpeedSpin()->GetValue(), 50.0);
        Undo(2);
    }
}

void TestClipSpeed::testChangeClipSpeedWithOffsetAndLength()
{
    StartTestSuite();
    TimelineZoomIn(2);
    Unlink(VideoClip(0, 4));
    TimelineDeleteClip(AudioClip(0, 4));
    TimelineTrimRight(VideoClip(0, 4), -50, false);
    TimelineTrimLeft(VideoClip(0, 4), 50, false); // Note: the trim adds an empty clip, thus the clip must now be indexed with '5'


    ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
    model::VideoFramePtr referenceFirstFrame{ FirstFrame(VideoClip(0, 5)) };
    model::VideoFramePtr referenceLAstFrame{ LastFrame(VideoClip(0, 5)) };

    {
        StartTest("Speed decrease: Enlarging clip allowed, begin and end frames stay the same");
        TimelineLeftClick(Center(VideoClip(0,5)));
        ASSERT(DetailsView(VideoClip(0,5)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
        ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1,100));
        ASSERT_EQUALS(DetailsClipView()->getSpeedSpin()->GetValue(), 0.01);
        ASSERT_EQUALS(DetailsClipView()->getSpeedSlider()->GetValue(), 100);
        ASSERT_EQUALS(VideoClip(0, 8)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
        StartTest("Speed decrease: Verify begin frame the same");
        Scrub(LeftPixel(VideoClip(0,5)), LeftPixel(VideoClip(0,5)) + 10);
        ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5)));
        StartTest("Speed decrease: Verify end frame the same");
        Scrub(RightPixel(VideoClip(0,5)) - 10, RightPixel(VideoClip(0,5)));
        ASSERT_FRAMES_EQUAL(referenceLAstFrame, LastFrame(VideoClip(0, 5)));
        Undo();
        ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1,1));
    }
    {
        StartTest("Speed increase: Reducing clip allowed, begin and end frames stay the same");
        TimelineLeftClick(Center(VideoClip(0,5)));
        ASSERT(DetailsView(VideoClip(0,5)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 10100);
        ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(2,1));
        ASSERT_EQUALS(DetailsClipView()->getSpeedSpin()->GetValue(), 2.00);
        ASSERT_EQUALS(VideoClip(0, 8)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
        StartTest("Speed increase: Verify begin frame the same");
        Scrub(LeftPixel(VideoClip(0,5)), LeftPixel(VideoClip(0,5)) + 10);
        ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5))); // Ensure first frame still the same
        StartTest("Speed increase: Verify end frame the same");
        Scrub(RightPixel(VideoClip(0,5)) - 10, RightPixel(VideoClip(0,5)));
        // NOT: ASSERT_FRAMES_EQUAL(referenceLAstFrame, LastFrame(VideoClip(0, 5))); -- todo why is the last frame in the faster clip, always the last - 2 of the original clip?
        Undo();
        ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1,1));
    }
    Undo(4);
}

void TestClipSpeed::testChangeClipSpeedWithInTransition()
{
    StartTestSuite();
    TimelineZoomIn(3);
    Unlink(VideoClip(0, 4));
    TimelineDeleteClip(AudioClip(0, 4));

    MakeInTransitionAfterClip preparation(3); // This makes the tested clip have number 5
    ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
    model::VideoFramePtr referenceFirstFrame{ FirstFrame(VideoClip(0, 5)) };
    model::VideoFramePtr referenceLAstFrame{ LastFrame(VideoClip(0, 5)) };

// todo gdi calls in non main thread cause crash: thumbnail being generated during scrubbing....
#ifndef __GNUC__
    {
        StartTest("Speed decrease: Enlarging clip allowed, begin and end frames stay the same");
        TimelineLeftClick(Center(VideoClip(0,5)));
        ASSERT(DetailsView(VideoClip(0,5)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
        Scrub(LeftPixel(VideoClip(0,5)), LeftPixel(VideoClip(0,5)) + 10);        
        ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1,100));
        ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
        StartTest("Speed decrease: Verify begin frame the same");
        Scrub(LeftPixel(VideoClip(0,5)), LeftPixel(VideoClip(0,5)) + 10);
        ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5)));
        StartTest("Speed decrease: Verify end frame the same");
        Scrub(RightPixel(VideoClip(0,5)) - 10, RightPixel(VideoClip(0,5)));
        ASSERT_FRAMES_EQUAL(referenceLAstFrame, LastFrame(VideoClip(0, 5)));
        Undo();
        ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1,1));
        ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
    }
    {
        StartTest("Speed increase: Reducing clip not allowed (need enough frames for transition)");
        TimelineLeftClick(Center(VideoClip(0,5)));
        ASSERT(DetailsView(VideoClip(0,5)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 10101);
        ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1,1));
        ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
    }
    {
        StartTest("Speed increase: Reducing clip allowed (when there's enough room), begin and end frames stay the same");
        TimelineTrimTransitionRightClipBegin(VideoClip(0,4),100);
        TimelineLeftClick(Center(VideoClip(0,5)));
        ASSERT(DetailsView(VideoClip(0,5)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 10100);
        ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(2,1));
        ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
        StartTest("Speed increase: Verify begin frame the same");
        Scrub(LeftPixel(VideoClip(0,5)), LeftPixel(VideoClip(0,5)) + 10);
        // NOT: ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5))); // Not always possible when increasing speed
        StartTest("Speed increase: Verify end frame the same");
        Scrub(RightPixel(VideoClip(0,5)) - 10, RightPixel(VideoClip(0,5)));
        // NOT: ASSERT_FRAMES_EQUAL(referenceLAstFrame, LastFrame(VideoClip(0, 5))); // Not always possible when increasing speed
        Undo();
        ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1,1));
        Undo();
        ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
    }
#endif
}

void TestClipSpeed::testChangeClipSpeedWithInOutTransitionBefore()
{
    StartTestSuite();
    TimelineZoomIn(3);
    Unlink(VideoClip(0, 4));
    TimelineDeleteClip(AudioClip(0, 4));

// todo gdi calls in non main thread cause crash: thumbnail being generated during scrubbing....
#ifndef __GNUC__
    MakeInOutTransitionAfterClip preparation(3); // This makes the tested clip have number 5
    ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
    model::VideoFramePtr referenceFirstFrame{ FirstFrame(VideoClip(0, 5)) };
    model::VideoFramePtr referenceLAstFrame{ LastFrame(VideoClip(0, 5)) };

    {
        StartTest("Speed decrease: Enlarging clip allowed, begin and end frames stay the same");
        TimelineLeftClick(Center(VideoClip(0,5)));
        ASSERT(DetailsView(VideoClip(0,5)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
        ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1,100));
        ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
        StartTest("Speed decrease: Verify begin frame the same");
        Scrub(LeftPixel(VideoClip(0,5)), LeftPixel(VideoClip(0,5)) + 10);
        ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5)));
        StartTest("Speed decrease: Verify end frame the same");
        Scrub(RightPixel(VideoClip(0,5)) - 10, RightPixel(VideoClip(0,5)));
        ASSERT_FRAMES_EQUAL(referenceLAstFrame, LastFrame(VideoClip(0, 5)));
        Undo();
        ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1,1));
        ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
    }
    {
        StartTest("Speed increase: Reducing clip not allowed (need enough frames for transition)");
        TimelineTrimTransitionRightClipBegin(VideoClip(0,4), -200); // Ensure that the entire 'clip offset' is required by the transition (extend as much as possible)
        TimelineLeftClick(Center(VideoClip(0,5)));
        ASSERT(DetailsView(VideoClip(0,5)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 10101);
        ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1,1));
        Undo();
        ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
    }
    {
        StartTest("Speed increase: Reducing clip allowed (when there's enough room), begin and end frames stay the same");
        // NOT: TimelineTrimTransitionRightClipBegin(VideoClip(0,4), 100); -- for creating the in-out-transition already a bit 'extra' is trimmed away.
        TimelineLeftClick(Center(VideoClip(0,5)));
        ASSERT(DetailsView(VideoClip(0,5)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 10100);
        ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(2,1));
        ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
        StartTest("Speed increase: Verify begin frame the same");
        Scrub(LeftPixel(VideoClip(0,5)), LeftPixel(VideoClip(0,5)) + 10);
        // NOT: ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5))); // Not always possible when increasing speed
        StartTest("Speed increase: Verify end frame the same");
        Scrub(RightPixel(VideoClip(0,5)) - 10, RightPixel(VideoClip(0,5)));
        // NOT: ASSERT_FRAMES_EQUAL(referenceLAstFrame, LastFrame(VideoClip(0, 5))); // Not always possible when increasing speed
        Undo();
        ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1,1));
        ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
    }
#endif
}

void TestClipSpeed::testChangeClipSpeedWithOutTransition()
{
    StartTestSuite();
    TimelineZoomIn(3);
    Unlink(VideoClip(0, 4));
    TimelineDeleteClip(AudioClip(0, 4));

    MakeOutTransitionAfterClip preparation(4);
    ASSERT_EQUALS(getVideoClip(VideoClip(0, 4))->getSpeed(), rational64(1, 1));
    model::VideoFramePtr referenceFirstFrame{ FirstFrame(VideoClip(0, 4)) };
    model::VideoFramePtr referenceLAstFrame{ LastFrame(VideoClip(0, 4)) };

// todo gdi calls in non main thread cause crash: thumbnail being generated during scrubbing....
#ifndef __GNUC__
    {
        StartTest("Speed decrease: Enlarging clip allowed, begin and end frames stay the same");
        TimelineLeftClick(Center(VideoClip(0,4)));
        ASSERT(DetailsView(VideoClip(0,4)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
        ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1,100));
        ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
        StartTest("Speed decrease: Verify begin frame the same");
        Scrub(LeftPixel(VideoClip(0,4)), LeftPixel(VideoClip(0,4)) + 10);
        ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 4)));
        StartTest("Speed decrease: Verify end frame the same");
        Scrub(RightPixel(VideoClip(0,4)) - 10, RightPixel(VideoClip(0,4)));
        ASSERT_FRAMES_EQUAL(referenceLAstFrame, LastFrame(VideoClip(0, 4)));
        Undo();
        ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1,1));
        ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
    }
    {
        StartTest("Speed increase: Reducing clip not allowed (need enough frames for transition)");
        TimelineLeftClick(Center(VideoClip(0,4)));
        ASSERT(DetailsView(VideoClip(0,4)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 10101);
        ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1,1));
        ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
    }
    {
        StartTest("Speed increase: Reducing clip allowed (when there's enough room), begin and end frames stay the same");
        TimelineTrimTransitionLeftClipEnd(VideoClip(0,5),-100);
        TimelineLeftClick(Center(VideoClip(0,4)));
        ASSERT(DetailsView(VideoClip(0,4)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 10100);
        ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(2,1));
        ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
        StartTest("Speed increase: Verify begin frame the same");
        Scrub(LeftPixel(VideoClip(0,4)), LeftPixel(VideoClip(0,4)) + 10);
        // NOT: ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5))); // Not always possible when increasing speed
        StartTest("Speed increase: Verify end frame the same");
        Scrub(RightPixel(VideoClip(0,4)) - 10, RightPixel(VideoClip(0,4)));
        // NOT: ASSERT_FRAMES_EQUAL(referenceLAstFrame, LastFrame(VideoClip(0, 5))); // Not always possible when increasing speed
        Undo();
        ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1,1));
        Undo();
        ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
    }
#endif
}

void TestClipSpeed::testChangeClipSpeedWithInOutTransitionAfter()
{
    StartTestSuite();
    TimelineZoomIn(3);
    Unlink(VideoClip(0, 4));
    TimelineDeleteClip(AudioClip(0, 4));

// todo gdi calls in non main thread cause crash: thumbnail being generated during scrubbing....
#ifndef __GNUC__
    MakeInOutTransitionAfterClip preparation(4);
    ASSERT_EQUALS(getVideoClip(VideoClip(0, 4))->getSpeed(), rational64(1, 1));
    model::VideoFramePtr referenceFirstFrame{ FirstFrame(VideoClip(0, 4)) };
    model::VideoFramePtr referenceLAstFrame{ LastFrame(VideoClip(0, 4)) };

    {
        StartTest("Speed decrease: Enlarging clip allowed, begin and end frames stay the same");
        TimelineLeftClick(Center(VideoClip(0,4)));
        ASSERT(DetailsView(VideoClip(0,4)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
        ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1,100));
        ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
        StartTest("Speed decrease: Verify begin frame the same");
        Scrub(LeftPixel(VideoClip(0,4)), LeftPixel(VideoClip(0,4)) + 10);
        ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 4)));
        StartTest("Speed decrease: Verify end frame the same");
        Scrub(RightPixel(VideoClip(0,4)) - 10, RightPixel(VideoClip(0,4)));
        ASSERT_FRAMES_EQUAL(referenceLAstFrame, LastFrame(VideoClip(0, 4)));
        Undo();
        ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1,1));
        ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
    }
    {
        StartTest("Speed increase: Reducing clip not allowed (need enough frames for transition)");
        TimelineTrimTransitionLeftClipEnd(VideoClip(0,5), +200); // Ensure that the entire 'clip offset' is required by the transition (extend as much as possible)
        TimelineLeftClick(Center(VideoClip(0,4)));
        ASSERT(DetailsView(VideoClip(0,4)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 10101);
        ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1,1));
        Undo();
        ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
    }
    {
        StartTest("Speed increase: Reducing clip allowed (when there's enough room), begin and end frames stay the same");
        // NOT: TimelineTrimTransitionLeftClipEnd(VideoClip(0,5), -100);  -- for creating the in-out-transition already a bit 'extra' is trimmed away.
        TimelineLeftClick(Center(VideoClip(0,4)));
        ASSERT(DetailsView(VideoClip(0,4)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 10100);
        ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(2,1));
        ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
        StartTest("Speed increase: Verify begin frame the same");
        Scrub(LeftPixel(VideoClip(0,4)), LeftPixel(VideoClip(0,4)) + 10);
        // NOT: ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5))); // Not always possible when increasing speed
        StartTest("Speed increase: Verify end frame the same");
        Scrub(RightPixel(VideoClip(0,4)) - 10, RightPixel(VideoClip(0,4)));
        // NOT: ASSERT_FRAMES_EQUAL(referenceLAstFrame, LastFrame(VideoClip(0, 5))); // Not always possible when increasing speed
        Undo();
        ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1,1));
        ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
    }
#endif
}

void TestClipSpeed::testChangeClipSpeedWithAdjacentTransitions()
{
    StartTestSuite();
    TimelineZoomIn(3);
    Unlink(VideoClip(0, 4));
    TimelineDeleteClip(AudioClip(0, 4));
    ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 1));
    model::VideoFramePtr referenceFirstFrame{ FirstFrame(VideoClip(0, 4)) };
    model::VideoFramePtr referenceLAstFrame{ LastFrame(VideoClip(0, 4)) };

// todo gdi calls in non main thread cause crash: thumbnail being generated during scrubbing....
#ifndef __GNUC__
    {
        MakeOutTransitionAfterClip preparation(3);
        {
            StartTest("Speed decrease: Enlarging clip allowed, begin and end frames stay the same");
            TimelineLeftClick(Center(VideoClip(0, 5)));
            ASSERT(DetailsView(VideoClip(0, 5)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
            ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 100));
            ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
            StartTest("Speed decrease: Verify begin frame the same");
            Scrub(LeftPixel(VideoClip(0, 5)), LeftPixel(VideoClip(0, 5)) + 10);
            ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5)));
            StartTest("Speed decrease: Verify end frame the same");
            Scrub(RightPixel(VideoClip(0, 5)) - 10, RightPixel(VideoClip(0, 5)));
            ASSERT_FRAMES_EQUAL(referenceLAstFrame, LastFrame(VideoClip(0, 5)));
            Undo();
            ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
        {
            StartTest("Speed increase: Reducing clip allowed (enough room), begin and end frames stay the same");
            TimelineLeftClick(Center(VideoClip(0, 5)));
            ASSERT(DetailsView(VideoClip(0, 5)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 10100);
            ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(2, 1));
            ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
            StartTest("Speed increase: Verify begin frame the same");
            Scrub(LeftPixel(VideoClip(0, 5)), LeftPixel(VideoClip(0, 5)) + 10);
            ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 5)));
            StartTest("Speed increase: Verify end frame the same");
            Scrub(RightPixel(VideoClip(0, 5)) - 10, RightPixel(VideoClip(0, 5)));
            // NOT: ASSERT_FRAMES_EQUAL(referenceLAstFrame, LastFrame(VideoClip(0, 5))); // Not always possible when increasing speed
            Undo();
            ASSERT_CLIP_SPEED(VideoClip(0, 5), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
    }
    {
        MakeInTransitionAfterClip preparation(4);
        {
            StartTest("Speed decrease: Enlarging clip allowed, begin and end frames stay the same");
            TimelineLeftClick(Center(VideoClip(0, 4)));
            ASSERT(DetailsView(VideoClip(0, 4)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 100); // Minimum speed is maximum length increase
            ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 100));
            ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
            StartTest("Speed decrease: Verify begin frame the same");
            Scrub(LeftPixel(VideoClip(0, 4)), LeftPixel(VideoClip(0, 4)) + 10);
            ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 4)));
            StartTest("Speed decrease: Verify end frame the same");
            Scrub(RightPixel(VideoClip(0, 4)) - 10, RightPixel(VideoClip(0, 4)));
            ASSERT_FRAMES_EQUAL(referenceLAstFrame, LastFrame(VideoClip(0, 4)));
            Undo();
            ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
        {
            StartTest("Speed increase: Reducing clip allowed (enough room), begin and end frames stay the same");
            TimelineLeftClick(Center(VideoClip(0, 4)));
            ASSERT(DetailsView(VideoClip(0, 4)));
            ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
            ASSERT(DetailsClipView()->getSpeedSpin()->IsEnabled());
            SetValue(DetailsClipView()->getSpeedSlider(), 10100);
            ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(2, 1));
            ASSERT_EQUALS(VideoClip(0, 7)->getLeftPts(), AudioClip(0, 6)->getLeftPts()); // Verify proper shifting
            StartTest("Speed increase: Verify begin frame the same");
            Scrub(LeftPixel(VideoClip(0, 4)), LeftPixel(VideoClip(0, 4)) + 10);
            ASSERT_FRAMES_EQUAL(referenceFirstFrame, FirstFrame(VideoClip(0, 4)));
            StartTest("Speed increase: Verify end frame the same");
            Scrub(RightPixel(VideoClip(0, 4)) - 10, RightPixel(VideoClip(0, 4)));
            // NOT: ASSERT_FRAMES_EQUAL(referenceLAstFrame, LastFrame(VideoClip(0, 5))); // Not always possible when increasing speed
            Undo();
            ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 1));
            ASSERT_HISTORY_END(gui::timeline::cmd::CreateTransition); // Verify that the command is not in the undo history
        }
    }
#endif
}

} // namespace
