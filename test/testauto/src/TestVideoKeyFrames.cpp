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

#include "TestVideoKeyFrames.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestVideoKeyFrames::setUp()
{
    mProjectFixture.init();
}

void TestVideoKeyFrames::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestVideoKeyFrames::testAddKeyFrames()
{
    StartTestSuite();
    TimelineZoomIn(2);
    pixel KeyFrame2Pixel = HCenter(VideoClip(0, 4));
    pixel KeyFrame1Pixel = KeyFrame2Pixel - 50;
    pixel KeyFrame3Pixel = KeyFrame2Pixel + 50;
    TimelineSelectClips({ VideoClip(0,4) });
    {
        StartTest("Cursor before clip: All keys disabled.");
        TimelinePositionCursor(HCenter(VideoClip(0, 1)));
        ASSERT_ZERO(getVideoClip(VideoClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT(DetailsView(VideoClip(0, 4)).VHome(false).VPrev(false).VNext(false).VEnd(false).VAdd(false).VRemove(false).VCount(0));
    }
    {
        StartTest("Cursor after clip: All keys disabled.");
        TimelinePositionCursor(HCenter(VideoClip(0, 6)));
        ASSERT_ZERO(getVideoClip(VideoClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT(DetailsView(VideoClip(0, 4)).VHome(false).VPrev(false).VNext(false).VEnd(false).VAdd(false).VRemove(false).VCount(0));
    }
    {
        StartTest("Cursor inside clip: Add button enabled.");
        TimelinePositionCursor(HCenter(VideoClip(0, 4)));
        ASSERT_ZERO(getVideoClip(VideoClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT(DetailsView(VideoClip(0, 4)).VHome(false).VPrev(false).VNext(false).VEnd(false).VAdd(true).VRemove(false).VCount(0));
    }
    {
        StartTest("Add first key frame.");
        TimelinePositionCursor(KeyFrame2Pixel);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        ASSERT_EQUALS(1, getVideoClip(VideoClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT(DetailsView(VideoClip(0, 4)).VHome(false).VPrev(false).VNext(false).VEnd(false).VAdd(false).VRemove(true).VCount(1));
        ASSERT_HISTORY_END(cmd::ProjectViewCreateSequence)(gui::timeline::cmd::EditClipDetails);
    }
    {
        StartTest("Position after first key frame.");
        TimelinePositionCursor(KeyFrame2Pixel + 50);
        ASSERT_EQUALS(1, getVideoClip(VideoClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT(DetailsView(VideoClip(0, 4)).VHome(true).VPrev(true).VNext(false).VEnd(false).VAdd(true).VRemove(false).VCount(1));
    }
    {
        StartTest("Position before first key frame.");
        TimelinePositionCursor(KeyFrame2Pixel - 50);
        ASSERT_EQUALS(1, getVideoClip(VideoClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT(DetailsView(VideoClip(0, 4)).VHome(false).VPrev(false).VNext(true).VEnd(true).VAdd(true).VRemove(false).VCount(1));
    }
    {
        StartTest("Multiple key frames.");
        TimelinePositionCursor(KeyFrame1Pixel);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails);
        TimelinePositionCursor(KeyFrame3Pixel);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails);
        ASSERT_EQUALS(3, getVideoClip(VideoClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT_EQUALS(3, DetailsClipView()->getVideoKeyFrameButtonCount());
    }
    {
        StartTest("Use video key frames buttons and give frames specific parameters. Key frame 1: Rotation.");
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFrameButton(0));
        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(0).Opacity(255).Scaling(model::VideoScalingFitToFill).ScalingFactor(1).Alignment(model::VideoAlignmentCenter).Position(wxPoint{ 0,0 }).Rotation(0));
        ASSERT(DetailsView(VideoClip(0, 4)).KeyFrameIndex(0).Opacity(255).Scaling(model::VideoScalingFitToFill).ScalingFactor(1).Alignment(model::VideoAlignmentCenter).Position(wxPoint{ 0,0 }).Rotation(0));
        SetValue(DetailsClipView()->getRotationSlider(), 9000); // 90 degrees
        ASSERT_HISTORY_END
            (gui::timeline::cmd::EditClipDetails) // Add key frame 1
            (gui::timeline::cmd::EditClipDetails) // Add key frame 2
            (gui::timeline::cmd::EditClipDetails) // Add key frame 3
            (gui::timeline::cmd::EditClipDetails);// Change rotation
        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(0).Rotation(90));
        ASSERT(DetailsView(VideoClip(0, 4)).Rotation(90));
    }
    {
        StartTest("Use video key frames buttons and give frames specific parameters. Key frame 2: Scaling.");
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFrameButton(1));
        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(1).Opacity(255).Scaling(model::VideoScalingFitToFill).ScalingFactor(1).Alignment(model::VideoAlignmentCenter).Position(wxPoint{ 0,0 }).Rotation(0));
        ASSERT(DetailsView(VideoClip(0, 4)).KeyFrameIndex(1).Opacity(255).Scaling(model::VideoScalingFitToFill).ScalingFactor(1).Alignment(model::VideoAlignmentCenter).Position(wxPoint{ 0,0 }).Rotation(0));
        SetValue(DetailsClipView()->getScalingSlider(), 5000); // scale to 50%
        ASSERT_HISTORY_END
            (gui::timeline::cmd::EditClipDetails) // Add key frame 1
            (gui::timeline::cmd::EditClipDetails) // Add key frame 2
            (gui::timeline::cmd::EditClipDetails) // Add key frame 3
            (gui::timeline::cmd::EditClipDetails) // Change rotation
            (gui::timeline::cmd::EditClipDetails);// Change scaling
        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(1).Scaling(model::VideoScalingCustom).ScalingFactor(rational64{ 1,2 }));
        ASSERT(DetailsView(VideoClip(0, 4)).Scaling(model::VideoScalingCustom).ScalingFactor(rational64{ 1,2 }));

    }
    {
        StartTest("Use video key frames buttons and give frames specific parameters. Key frame 3: Opacity.");
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFrameButton(2));
        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(2).Opacity(255).Scaling(model::VideoScalingFitToFill).ScalingFactor(1).Alignment(model::VideoAlignmentCenter).Position(wxPoint{ 0,0 }).Rotation(0));
        ASSERT(DetailsView(VideoClip(0, 4)).KeyFrameIndex(2).Opacity(255).Scaling(model::VideoScalingFitToFill).ScalingFactor(1).Alignment(model::VideoAlignmentCenter).Position(wxPoint{ 0,0 }).Rotation(0));
        SetValue(DetailsClipView()->getOpacitySlider(), 128); // 50% opaque
        ASSERT_HISTORY_END
            (gui::timeline::cmd::EditClipDetails) // Add key frame 1
            (gui::timeline::cmd::EditClipDetails) // Add key frame 2
            (gui::timeline::cmd::EditClipDetails) // Add key frame 3
            (gui::timeline::cmd::EditClipDetails) // Change rotation
            (gui::timeline::cmd::EditClipDetails)// Change scaling
            (gui::timeline::cmd::EditClipDetails); // Change opacity
        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(2).Opacity(128));
        ASSERT(DetailsView(VideoClip(0, 4)).Opacity(128));
    }
    {
        StartTest("Interpolation before first key frame");
        TimelinePositionCursor(KeyFrame1Pixel - 20); // Before key frame 1
        ASSERT(DetailsView(VideoClip(0, 4)).Rotation(90));
    }
    {
        StartTest("Interpolation between key frames");
        TimelinePositionCursor((KeyFrame1Pixel + KeyFrame2Pixel) / 2); // Between key frames 1 and 2
        ASSERT(DetailsView(VideoClip(0, 4)).Rotation(45));
        ASSERT(DetailsView(VideoClip(0, 4)).Scaling(model::VideoScalingCustom).ScalingFactor(rational64{ 3, 4 }));
        TimelinePositionCursor((KeyFrame2Pixel + KeyFrame3Pixel) / 2); // Between key frames 2 and 3
        ASSERT(DetailsView(VideoClip(0, 4)).Scaling(model::VideoScalingCustom).ScalingFactor(rational64{ 3, 4 }));
        ASSERT(DetailsView(VideoClip(0, 4)).Opacity(192));
    }
    {
        StartTest("Interpolation after last key frame");
        TimelinePositionCursor(KeyFrame3Pixel + 10); // After key frame 3
        ASSERT(DetailsView(VideoClip(0, 4)).Opacity(128));
    }
}

void TestVideoKeyFrames::testRemoveKeyFrames()
{
    StartTestSuite();
    TimelineZoomIn(2);
    pixel KeyFrame2Pixel{ HCenter(VideoClip(0, 4)) };
    pixel KeyFrame1Pixel{ KeyFrame2Pixel - 100 };
    pixel KeyFrame3Pixel{ KeyFrame2Pixel + 100 };
    TimelineSelectClips({ VideoClip(0,4) });

    auto ASSERT_KEYFRAMES = [&](std::vector<pixel> positions)
    {
        model::VideoClipPtr videoclip{ getVideoClip(VideoClip(0,4)) };
        std::map<pts, model::KeyFramePtr> keyFrames{ videoclip->getKeyFramesOfPerceivedClip() };
        ASSERT_EQUALS(keyFrames.size(), positions.size())(keyFrames)(positions);
        for (pts p : positions)
        {
            ASSERT_MAP_CONTAINS(keyFrames, getTimeline().getZoom().pixelsToPts(p) - videoclip->getPerceivedLeftPts());
        }
    };

    {
        StartTest("Prepare key frames.");
        TimelinePositionCursor(KeyFrame1Pixel);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        TimelinePositionCursor(KeyFrame2Pixel);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        TimelinePositionCursor(KeyFrame3Pixel);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        ASSERT_EQUALS(3, getVideoClip(VideoClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT_EQUALS(3, DetailsClipView()->getVideoKeyFrameButtonCount());
        ASSERT_KEYFRAMES({ KeyFrame1Pixel, KeyFrame2Pixel, KeyFrame3Pixel });
    }
    {
        StartTest("Remove first key frame");
        TimelinePositionCursor(KeyFrame1Pixel);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesRemoveButton());
        ASSERT_KEYFRAMES({ KeyFrame2Pixel, KeyFrame3Pixel });
        Undo();
        ASSERT_EQUALS(3, getVideoClip(VideoClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT_EQUALS(3, DetailsClipView()->getVideoKeyFrameButtonCount());
    }
    {
        StartTest("Remove middle key frame");
        TimelinePositionCursor(KeyFrame2Pixel);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesRemoveButton());
        ASSERT_KEYFRAMES({ KeyFrame1Pixel, KeyFrame3Pixel });
        Undo();
        ASSERT_EQUALS(3, getVideoClip(VideoClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT_EQUALS(3, DetailsClipView()->getVideoKeyFrameButtonCount());
    }
    {
        StartTest("Remove last key frame");
        TimelinePositionCursor(KeyFrame3Pixel);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesRemoveButton());
        ASSERT_KEYFRAMES({ KeyFrame1Pixel, KeyFrame2Pixel });
        Undo();
        ASSERT_EQUALS(3, getVideoClip(VideoClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT_EQUALS(3, DetailsClipView()->getVideoKeyFrameButtonCount());
    }
}

void TestVideoKeyFrames::testNavigation()
{
    StartTestSuite();
    TimelineZoomIn(2);
    pixel KeyFrame2Pixel{ HCenter(VideoClip(0, 4)) };
    pixel KeyFrame1Pixel{ KeyFrame2Pixel - 50 };
    pixel KeyFrame3Pixel{ KeyFrame2Pixel + 50 };
    pixel BeforeKeyFrame1{ KeyFrame1Pixel - 10 };
    pixel BetweenKeyFrame1AndKeyFrame2{ KeyFrame2Pixel - 40 };
    pixel BetweenKeyFrame2AndKeyFrame3{ KeyFrame3Pixel - 40 };
    pixel AfterKeyFrame3{ KeyFrame3Pixel + 10 };
    TimelineSelectClips({ VideoClip(0,4) });
    {
        StartTest("Prepare key frames.");
        TimelinePositionCursor(KeyFrame1Pixel);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        TimelinePositionCursor(KeyFrame2Pixel);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        TimelinePositionCursor(KeyFrame3Pixel);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        ASSERT_EQUALS(3, getVideoClip(VideoClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT(DetailsView(VideoClip(0, 4)).VCount(3));
        ASSERT_EQUALS(3, DetailsClipView()->getVideoKeyFrameButtonCount());
    }
    auto CHECK_BUTTON = [&](pixel from, wxButton* button, pixel to, size_t keyFrameIndex)
    {
        TimelinePositionCursor(from);
        ButtonTriggerPressed(button);
        ASSERT(DetailsView(VideoClip(0, 4)).KeyFrameIndex(keyFrameIndex));
    };
    {
        StartTest("Before first key frame");
        TimelinePositionCursor(BeforeKeyFrame1);
        ASSERT(DetailsView(VideoClip(0, 4)).VHome(false).VPrev(false).VNext(true).VEnd(true).VAdd(true).VRemove(false).VCount(3).NoKeyframe());
        CHECK_BUTTON(BeforeKeyFrame1, DetailsClipView()->getVideoKeyFramesNextButton(), KeyFrame1Pixel, 0);
        CHECK_BUTTON(BeforeKeyFrame1, DetailsClipView()->getVideoKeyFramesEndButton(), KeyFrame3Pixel, 2);
    }
    {
        StartTest("On first key frame");
        TimelinePositionCursor(KeyFrame1Pixel);
        ASSERT(DetailsView(VideoClip(0, 4)).VHome(false).VPrev(false).VNext(true).VEnd(true).VAdd(false).VRemove(true).VCount(3).KeyFrameIndex(0));
        CHECK_BUTTON(KeyFrame1Pixel, DetailsClipView()->getVideoKeyFramesNextButton(), KeyFrame2Pixel, 1);
        CHECK_BUTTON(KeyFrame1Pixel, DetailsClipView()->getVideoKeyFramesEndButton(), KeyFrame3Pixel, 2);
    }
    {
        StartTest("Between key frame 1 and key frame 2");
        TimelinePositionCursor(BetweenKeyFrame1AndKeyFrame2);
        ASSERT(DetailsView(VideoClip(0, 4)).VHome(true).VPrev(true).VNext(true).VEnd(true).VAdd(true).VRemove(false).VCount(3).NoKeyframe());
        CHECK_BUTTON(BetweenKeyFrame1AndKeyFrame2, DetailsClipView()->getVideoKeyFramesHomeButton(), KeyFrame1Pixel, 0);
        CHECK_BUTTON(BetweenKeyFrame1AndKeyFrame2, DetailsClipView()->getVideoKeyFramesPrevButton(), KeyFrame1Pixel, 0);
        CHECK_BUTTON(BetweenKeyFrame1AndKeyFrame2, DetailsClipView()->getVideoKeyFramesNextButton(), KeyFrame2Pixel, 1);
        CHECK_BUTTON(BetweenKeyFrame1AndKeyFrame2, DetailsClipView()->getVideoKeyFramesEndButton(), KeyFrame3Pixel, 2);
    }
    {
        StartTest("On key frame 2");
        TimelinePositionCursor(KeyFrame2Pixel);
        ASSERT(DetailsView(VideoClip(0, 4)).VHome(true).VPrev(true).VNext(true).VEnd(true).VAdd(false).VRemove(true).VCount(3).KeyFrameIndex(1));
        CHECK_BUTTON(KeyFrame2Pixel, DetailsClipView()->getVideoKeyFramesHomeButton(), KeyFrame1Pixel, 0);
        CHECK_BUTTON(KeyFrame2Pixel, DetailsClipView()->getVideoKeyFramesPrevButton(), KeyFrame1Pixel, 0);
        CHECK_BUTTON(KeyFrame2Pixel, DetailsClipView()->getVideoKeyFramesNextButton(), KeyFrame3Pixel, 2);
        CHECK_BUTTON(KeyFrame2Pixel, DetailsClipView()->getVideoKeyFramesEndButton(), KeyFrame3Pixel, 2);
    }
    {
        StartTest("Between key frame 2 and key frame 3");
        TimelinePositionCursor(BetweenKeyFrame2AndKeyFrame3);
        ASSERT(DetailsView(VideoClip(0, 4)).VHome(true).VPrev(true).VNext(true).VEnd(true).VAdd(true).VRemove(false).VCount(3).NoKeyframe());
        CHECK_BUTTON(BetweenKeyFrame2AndKeyFrame3, DetailsClipView()->getVideoKeyFramesHomeButton(), KeyFrame1Pixel, 0);
        CHECK_BUTTON(BetweenKeyFrame2AndKeyFrame3, DetailsClipView()->getVideoKeyFramesPrevButton(), KeyFrame2Pixel, 1);
        CHECK_BUTTON(BetweenKeyFrame2AndKeyFrame3, DetailsClipView()->getVideoKeyFramesNextButton(), KeyFrame3Pixel, 2);
        CHECK_BUTTON(BetweenKeyFrame2AndKeyFrame3, DetailsClipView()->getVideoKeyFramesEndButton(), KeyFrame3Pixel, 2);
    }
    {
        StartTest("On key frame 3");
        TimelinePositionCursor(KeyFrame3Pixel);
        ASSERT(DetailsView(VideoClip(0, 4)).VHome(true).VPrev(true).VNext(false).VEnd(false).VAdd(false).VRemove(true).VCount(3).KeyFrameIndex(2));
        CHECK_BUTTON(KeyFrame3Pixel, DetailsClipView()->getVideoKeyFramesHomeButton(), KeyFrame1Pixel, 0);
        CHECK_BUTTON(KeyFrame3Pixel, DetailsClipView()->getVideoKeyFramesPrevButton(), KeyFrame2Pixel, 1);
    }
    {
        StartTest("After key frame 3");
        TimelinePositionCursor(AfterKeyFrame3);
        ASSERT(DetailsView(VideoClip(0, 4)).VHome(true).VPrev(true).VNext(false).VEnd(false).VAdd(true).VRemove(false).VCount(3).NoKeyframe());
        CHECK_BUTTON(AfterKeyFrame3, DetailsClipView()->getVideoKeyFramesHomeButton(), KeyFrame1Pixel, 0);
        CHECK_BUTTON(AfterKeyFrame3, DetailsClipView()->getVideoKeyFramesPrevButton(), KeyFrame3Pixel, 2);
    }
}

void TestVideoKeyFrames::testSelectKeyFrameInTimeline()
{
    StartTestSuite();
    TimelineZoomIn(2);
    pixel KeyFrame2Pixel{ HCenter(VideoClip(0, 4)) };
    pixel KeyFrame1Pixel{ KeyFrame2Pixel - 100 };
    pixel KeyFrame3Pixel{ KeyFrame2Pixel + 100 };
    TimelineSelectClips({ VideoClip(0,4) });
    {
        StartTest("Prepare key frames.");
        TimelinePositionCursor(KeyFrame1Pixel);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        TimelinePositionCursor(KeyFrame2Pixel);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        TimelinePositionCursor(KeyFrame3Pixel);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        ASSERT_EQUALS(3, getVideoClip(VideoClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT_EQUALS(3, DetailsClipView()->getVideoKeyFrameButtonCount());
        TimelineSelectClips({});
        ASSERT(DetailsView(nullptr));
    }
    {
        StartTest("Select first key frame");
        TimelineLeftClick(wxPoint(KeyFrame1Pixel, BottomPixel(VideoClip(0, 4)) - 2));
        ASSERT(DetailsView(VideoClip(0,4)).KeyFrameIndex(0));
        TimelineSelectClips({});
        ASSERT(DetailsView(nullptr));
    }
    {
        StartTest("Select middle key frame");
        TimelineLeftClick(wxPoint(KeyFrame2Pixel, BottomPixel(VideoClip(0, 4)) - 2));
        ASSERT(DetailsView(VideoClip(0, 4)).KeyFrameIndex(1));
        TimelineSelectClips({});
        ASSERT(DetailsView(nullptr));
    }
    {
        StartTest("Select last key frame");
        TimelineLeftClick(wxPoint(KeyFrame3Pixel, BottomPixel(VideoClip(0, 4)) - 2));
        ASSERT(DetailsView(VideoClip(0, 4)).KeyFrameIndex(2));
        TimelineSelectClips({});
        ASSERT(DetailsView(nullptr));
    }
}

void TestVideoKeyFrames::testRemoveLastKeyFrame()
{
    StartTestSuite();
    TimelineZoomIn(2);

    ASSERT(DefaultKeyFrame(VideoClip(0, 4)).Opacity(255).Scaling(model::VideoScalingFitToFill).ScalingFactor(1).Alignment(model::VideoAlignmentCenter).Position(wxPoint{ 0,0 }).Rotation(0));
    TimelinePositionCursor(HCenter(VideoClip(0, 4)));
    ASSERT(DetailsView(nullptr));
    TimelineSelectClips({ VideoClip(0,4) });
    ASSERT(DetailsView(VideoClip(0, 4)));
    ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());

    StartTest("Change key frame.");
    ASSERT_EQUALS(1, getVideoClip(VideoClip(0, 4))->getKeyFramesOfPerceivedClip().size());
    SetValue(DetailsClipView()->getRotationSlider(), 9000); // 90 degrees
    SetValue(DetailsClipView()->getScalingSlider(), 5000); // scale to 50%
    SetValue(DetailsClipView()->getOpacitySlider(), 128); // 50% opaque
    SetValue(DetailsClipView()->getPositionXSlider(), 111);
    SetValue(DetailsClipView()->getPositionYSlider(), 222);
    ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(0).Opacity(128).Scaling(model::VideoScalingCustom).ScalingFactor(rational64{ 1,2 }).Alignment(model::VideoAlignmentCustom).Position(wxPoint{ 111, 222 }).Rotation(90));

    StartTest("Delete key frame and verify default key frame updated");
    ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesRemoveButton());
    ASSERT(DefaultKeyFrame(VideoClip(0, 4)).Opacity(128).Scaling(model::VideoScalingCustom).ScalingFactor(rational64{ 1,2 }).Alignment(model::VideoAlignmentCustom).Position(wxPoint{ 111, 222 }).Rotation(90));
}

void TestVideoKeyFrames::testKeyFramesOnBeginAndEndOfClip()
{
    StartTestSuite();
    TimelineZoomIn(4);
    TimelineSelectClips({ VideoClip(0,1) });
    {
        StartTest("Prepare key frames.");
        TimelinePositionCursor(LeftPixel(VideoClip(0,1)));
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        ASSERT(DetailsView(VideoClip(0, 1)).VCount(1));
        TimelinePositionCursor(LeftPixel(VideoClip(0, 2)));
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        ASSERT(DetailsView(VideoClip(0, 1)).VCount(2));
        ASSERT(KeyFrame(VideoClip(0, 1)).KeyFrameIndex(0).KeyFrameOffset(0));
        ASSERT(KeyFrame(VideoClip(0, 1)).KeyFrameIndex(1).KeyFrameOffset(VideoClip(0,1)->getLength() - 1 + 1)); // +1: The rightmost key frame is AFTER the last key frame
    }
    {
        StartTest("Cursor after clip");
        TimelinePositionCursor(LeftPixel(VideoClip(0, 2)) + 5);
        ASSERT(DetailsView(VideoClip(0, 1)).VHome(true).VPrev(true).VNext(false).VEnd(false).VAdd(false).VRemove(true).VCount(2).KeyFrameIndex(1));
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesPrevButton());
        ASSERT(DetailsView(VideoClip(0, 1)).KeyFrameIndex(0));
    }
    {
        StartTest("Cursor before clip");
        TimelinePositionCursor(RightPixel(VideoClip(0, 0)) - 5);
        ASSERT(DetailsView(VideoClip(0, 1)).VHome(false).VPrev(false).VNext(true).VEnd(true).VAdd(false).VRemove(true).VCount(2).KeyFrameIndex(0));
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesNextButton());
        ASSERT(DetailsView(VideoClip(0, 1)).KeyFrameIndex(1));
    }
}

void TestVideoKeyFrames::testGetKeyFrameWithoutOffset()
{
    StartTestSuite();
    TimelineZoomIn(2);
    pixel InterpolatedPixel{ HCenter(VideoClip(0, 4)) };
    pixel KeyFrame1Pixel{ InterpolatedPixel - 50 };
    pixel KeyFrame2Pixel{ InterpolatedPixel + 50 };
    TimelineSelectClips({ VideoClip(0,4) });
    {
        StartTest("Prepare key frames.");
        TimelinePositionCursor(KeyFrame1Pixel);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        SetValue(DetailsClipView()->getRotationSpin(), -90.0);
        TimelinePositionCursor(KeyFrame2Pixel);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        SetValue(DetailsClipView()->getRotationSpin(), +90.0);
        ASSERT_EQUALS(2, getVideoClip(VideoClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT(DetailsView(VideoClip(0, 4)).VCount(2));
    }
    {
        StartTest("Get rotation for first key frame");
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFrameButton(0));
        ASSERT(DetailsView(VideoClip(0, 4)).KeyFrameIndex(0).Rotation(-90));
    }
    {
        StartTest("Get rotation before first key frame");
        TimelinePositionCursor(LeftPixel(VideoClip(0, 4)) + 5);
        ASSERT(DetailsView(VideoClip(0, 4)).Rotation(-90));
    }
    {
        StartTest("Get rotation for second key frame");
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFrameButton(1));
        ASSERT(DetailsView(VideoClip(0, 4)).KeyFrameIndex(1).Rotation(90));
    }
    {
        StartTest("Get rotation after second key frame");
        TimelinePositionCursor(RightPixel(VideoClip(0, 4)) - 5);
        ASSERT(DetailsView(VideoClip(0, 4)).Rotation(+90));
    }
    {
        StartTest("Get rotation for interpolated frame");
        TimelinePositionCursor(HCenter(VideoClip(0, 4)));
        ASSERT(DetailsView(VideoClip(0, 4)).Rotation(0));
    }
}

void TestVideoKeyFrames::testGetKeyFrameWithOffset()
{
    StartTestSuite();
    TimelineZoomIn(2);
    TimelineTrimLeft(VideoClip(0, 4), 50);
    pixel InterpolatedPixel{ HCenter(VideoClip(0, 4)) };
    pixel KeyFrame1Pixel{ InterpolatedPixel - 50 };
    pixel KeyFrame2Pixel{ InterpolatedPixel + 50 };
    TimelineSelectClips({ VideoClip(0,4) });
    {
        StartTest("Prepare key frames.");
        TimelinePositionCursor(KeyFrame1Pixel);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        SetValue(DetailsClipView()->getRotationSpin(), -90.0);
        TimelinePositionCursor(KeyFrame2Pixel);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        SetValue(DetailsClipView()->getRotationSpin(), +90.0);
        ASSERT_EQUALS(2, getVideoClip(VideoClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT(DetailsView(VideoClip(0, 4)).VCount(2));
    }
    {
        StartTest("Get rotation for first key frame");
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFrameButton(0));
        ASSERT(DetailsView(VideoClip(0, 4)).KeyFrameIndex(0).Rotation(-90));
    }
    {
        StartTest("Get rotation before first key frame");
        TimelinePositionCursor(LeftPixel(VideoClip(0, 4)) + 5);
        ASSERT(DetailsView(VideoClip(0, 4)).Rotation(-90));
    }
    {
        StartTest("Get rotation for second key frame");
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFrameButton(1));
        ASSERT(DetailsView(VideoClip(0, 4)).KeyFrameIndex(1).Rotation(90));
    }
    {
        StartTest("Get rotation after second key frame");
        TimelinePositionCursor(RightPixel(VideoClip(0, 4)) - 5);
        ASSERT(DetailsView(VideoClip(0, 4)).Rotation(+90));
    }
    {
        StartTest("Get rotation for interpolated frame");
        TimelinePositionCursor(HCenter(VideoClip(0, 4)));
        ASSERT(DetailsView(VideoClip(0, 4)).Rotation(0));
    }
}

void TestVideoKeyFrames::testGetKeyFrameWithTransition()
{
    StartTestSuite();
    TimelineZoomIn(2);
    MakeInOutTransitionAfterClip prepare(3);
    prepare.dontUndo();
    pixel InterpolatedPixel{ HCenter(VideoClip(0, 5)) };
    pixel KeyFrame1Pixel{ InterpolatedPixel - 50 };
    pixel KeyFrame2Pixel{ InterpolatedPixel + 50 };
    TimelineSelectClips({ VideoClip(0, 5) });
    {
        StartTest("Prepare key frames.");
        TimelinePositionCursor(KeyFrame1Pixel);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        SetValue(DetailsClipView()->getRotationSpin(), -90.0);
        TimelinePositionCursor(KeyFrame2Pixel);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        SetValue(DetailsClipView()->getRotationSpin(), +90.0);
        ASSERT_EQUALS(2, getVideoClip(VideoClip(0, 5))->getKeyFramesOfPerceivedClip().size());
        ASSERT(DetailsView(VideoClip(0, 5)).VCount(2));
    }
    {
        StartTest("Get rotation for first key frame");
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFrameButton(0));
        ASSERT(DetailsView(VideoClip(0, 5)).KeyFrameIndex(0).Rotation(-90));
    }
    {
        StartTest("Get rotation before first key frame");
        TimelinePositionCursor(LeftPixel(VideoClip(0, 4)) + 5);
        ASSERT(DetailsView(VideoClip(0, 5)).Rotation(-90));
    }
    {
        StartTest("Get rotation for second key frame");
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFrameButton(1));
        ASSERT(DetailsView(VideoClip(0, 5)).KeyFrameIndex(1).Rotation(90));
    }
    {
        StartTest("Get rotation after second key frame");
        TimelinePositionCursor(RightPixel(VideoClip(0, 5)) - 5);
        ASSERT(DetailsView(VideoClip(0, 5)).Rotation(+90));
    }
    {
        StartTest("Get rotation for interpolated frame");
        TimelinePositionCursor(HCenter(VideoClip(0, 5)));
        ASSERT(DetailsView(VideoClip(0, 5)).Rotation(0));
    }
}

void TestVideoKeyFrames::testChangeClipSpeed()
{
    StartTestSuite();
    TimelineZoomIn(4); // Zooming in required to ensure that the last 'center positioning' is at exactly the key frame (on the clip with an increased speed)

    StartTest("Make key frame in center");
    Unlink(VideoClip(0, 4));
    TimelineDeleteClip(AudioClip(0, 4));
    TimelinePositionCursor(HCenter(VideoClip(0, 4)));
    TimelineSelectClips({ VideoClip(0,4) });
    ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
    SetValue(DetailsClipView()->getRotationSlider(), 9000); // 90 degrees
    ASSERT_HISTORY_END
        (cmd::ProjectViewCreateAutoFolder)
        (cmd::ProjectViewCreateSequence)
        (gui::timeline::cmd::UnlinkClips)
        (gui::timeline::cmd::DeleteSelectedClips)
        (gui::timeline::cmd::EditClipDetails) // Add key frame
        (gui::timeline::cmd::EditClipDetails); // Rotate
    {
        StartTest("Increase speed and verify key frame");
        SetValue(DetailsClipView()->getSpeedSpin(), 10.0);
        ASSERT(DetailsView(VideoClip(0, 4)).NoKeyframe());
        ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(10, 1));
        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(0).Rotation(90));
        TimelinePositionCursor(HCenter(VideoClip(0, 4)));
        ASSERT(DetailsView(VideoClip(0, 4)).KeyFrameIndex(0));
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipSpeed);
        Undo();
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails);
    }
    {
        StartTest("Decrease speed and verify key frame");
        SetValue(DetailsClipView()->getSpeedSlider(), 5000); // Set speed to 0.5
        ASSERT(DetailsView(VideoClip(0, 4)).NoKeyframe());
        ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 2));
        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(0).Rotation(90));
        TimelinePositionCursor(HCenter(VideoClip(0, 4)));
        ASSERT(DetailsView(VideoClip(0, 4)).KeyFrameIndex(0));
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipSpeed);
        Undo();
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails);
    }
}

void TestVideoKeyFrames::testTrimAwayKeyFrames()
{
    StartTestSuite();
    TimelineZoomIn(2);
    pixel KeyFrame2Pixel = HCenter(VideoClip(0, 4));
    pixel KeyFrame1Pixel = KeyFrame2Pixel - 50;
    pixel KeyFrame3Pixel = KeyFrame2Pixel + 50;
    TimelineSelectClips({ VideoClip(0, 4) });
    TimelinePositionCursor(KeyFrame1Pixel);
    ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
    TimelinePositionCursor(KeyFrame2Pixel);
    ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
    TimelinePositionCursor(KeyFrame3Pixel);
    ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
    ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails);
    ASSERT_EQUALS(DetailsClipView()->getVideoKeyFrameButtonCount(), 3);
    {
        StartTest("Remove frame by begin trim");
        TimelineTrimLeft(VideoClip(0, 4), 100);
        ASSERT(DetailsView(VideoClip(0, 4)).VCount(2));
        ASSERT_HISTORY_END(gui::timeline::cmd::TrimClip);
    }
    {
        StartTest("Add frame by begin trim (impossible)");
        TimelineTrimLeft(VideoClip(0, 4), -100);
        ASSERT(DetailsView(VideoClip(0, 4)).VCount(2));
        ASSERT_HISTORY_END(gui::timeline::cmd::TrimClip)(gui::timeline::cmd::TrimClip);
    }
    {
        StartTest("Add frame again by Undoing");
        Undo(2);
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails);
        ASSERT(DetailsView(VideoClip(0, 4)).VCount(3));
    }
    {
        StartTest("Remove frame by end trim");
        TimelineTrimRight(VideoClip(0, 4), -100);
        ASSERT(DetailsView(VideoClip(0, 4)).VCount(2));
        ASSERT_HISTORY_END(gui::timeline::cmd::TrimClip);
    }
    {
        StartTest("Add frame by end trim (impossible)");
        TimelineTrimRight(VideoClip(0, 4), 100);
        ASSERT(DetailsView(VideoClip(0, 4)).VCount(2));
        ASSERT_HISTORY_END(gui::timeline::cmd::TrimClip)(gui::timeline::cmd::TrimClip);
    }
    {
        StartTest("Add frame again by Undoing");
        Undo(2);
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails);
        ASSERT(DetailsView(VideoClip(0, 4)).VCount(3));
    }
}

void TestVideoKeyFrames::testBeginOffset()
{
    StartTestSuite();
    TimelineTrimLeft(VideoClip(0, 3), 100);
    {
        StartTest("Zoom 2");
        TimelineZoomIn(2);
        TimelineSelectClips({ VideoClip(0,3) });
        TimelinePositionCursor(LeftPixel(VideoClip(0, 3)) + 50);
        TimelineKeyPress('v');
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        ASSERT(KeyFrame(VideoClip(0, 3)).KeyFrameIndex(0));
        ASSERT(DetailsView(VideoClip(0, 3)).VCount(1).KeyFrameIndex(0));
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesRemoveButton());
        ASSERT(KeyFrame(VideoClip(0, 3)));
        ASSERT(DetailsView(VideoClip(0, 3)).VCount(0).NoKeyframe());
    }
    {
        StartTest("Zoom 8");
        TimelineZoomIn(6);
        TimelineSelectClips({ VideoClip(0,3) });
        TimelinePositionCursor(LeftPixel(VideoClip(0, 3)) + 20);
        TimelineKeyPress('v');
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        ASSERT(KeyFrame(VideoClip(0, 3)).KeyFrameIndex(0));
        ASSERT(DetailsView(VideoClip(0, 3)).VCount(1).KeyFrameIndex(0));
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesRemoveButton());
        ASSERT(KeyFrame(VideoClip(0, 3)));
        ASSERT(DetailsView(VideoClip(0, 3)).NoKeyframe().VCount(0));
    }
}

void TestVideoKeyFrames::testInTransition()
{
    StartTestSuite();
    TimelineZoomIn(8);
    MakeInOutTransitionAfterClip prepare(3);
    prepare.dontUndo();
    {
        StartTest("Make two key frames under an in transition");
        TimelinePositionCursor(LeftPixel(VideoClip(0, 5)) - 3);
        TimelineKeyPress('v');
        TimelineSelectClips({ VideoClip(0, 5) });
        ASSERT(DetailsView(VideoClip(0,5)).VAdd(true));
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        ASSERT(DetailsView(VideoClip(0,5)).VAdd(false).VCount(1));
        TimelinePositionCursor(LeftPixel(VideoClip(0, 5)) - 12);
        ASSERT(DetailsView(VideoClip(0,5)).VAdd(true).VCount(1));
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        ASSERT(DetailsView(VideoClip(0,5)).VAdd(false).VCount(2));
    }
    {
        StartTest("Cursor position maps onto key frames");
        TimelinePositionCursor(LeftPixel(VideoClip(0, 5)) - 3);
        ASSERT(DetailsView(VideoClip(0,5)).KeyFrameIndex(1));
        TimelinePositionCursor(LeftPixel(VideoClip(0, 5)) - 12);
        ASSERT(DetailsView(VideoClip(0,5)).KeyFrameIndex(0));
    }
    {
        StartTest("Change key frames");

        ASSERT(KeyFrame(VideoClip(0, 5)).KeyFrameIndex(0).Position(wxPoint{ 0, 0 }));
        ASSERT(KeyFrame(VideoClip(0, 5)).KeyFrameIndex(1).Position(wxPoint{ 0, 0 }));

        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFrameButton(0));
        ASSERT(KeyFrame(VideoClip(0, 5)).KeyFrameIndex(0).Position(wxPoint{ 0, 0 }));
        ASSERT(KeyFrame(VideoClip(0, 5)).KeyFrameIndex(1).Position(wxPoint{ 0, 0 }));
        ASSERT(DetailsView(VideoClip(0,5)).KeyFrameIndex(0).Position(wxPoint{0,0}));

        SetValue(DetailsClipView()->getPositionXSpin(), -128);
        ASSERT(KeyFrame(VideoClip(0, 5)).KeyFrameIndex(0).Position(wxPoint{ -128, 0 }));
        ASSERT(KeyFrame(VideoClip(0, 5)).KeyFrameIndex(1).Position(wxPoint{ 0, 0 }));
        ASSERT(DetailsView(VideoClip(0,5)).KeyFrameIndex(0).Position(wxPoint{-128,0}));

        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFrameButton(1));
        ASSERT(KeyFrame(VideoClip(0, 5)).KeyFrameIndex(0).Position(wxPoint{ -128, 0 }));
        ASSERT(KeyFrame(VideoClip(0, 5)).KeyFrameIndex(1).Position(wxPoint{ 0, 0 }));
        ASSERT(DetailsView(VideoClip(0,5)).KeyFrameIndex(1).Position(wxPoint{0, 0}));

        SetValue(DetailsClipView()->getPositionYSpin(), 128);
        ASSERT(KeyFrame(VideoClip(0, 5)).KeyFrameIndex(0).Position(wxPoint{ -128, 0 }));
        ASSERT(KeyFrame(VideoClip(0, 5)).KeyFrameIndex(1).Position(wxPoint{ 0, 128 }));
        ASSERT(DetailsView(VideoClip(0,5)).KeyFrameIndex(1).Position(wxPoint{0, 128}));
    }
    {
        StartTest("Playback");
        TimelinePositionCursor(HCenter(VideoClip(0, 4)));
        Play(500);
    }
}

void TestVideoKeyFrames::testOutTransition()
{
    StartTestSuite();
    TimelineZoomIn(8);
    MakeInOutTransitionAfterClip prepare(4);
    prepare.dontUndo();
    {
        StartTest("Make two key frames under an out transition");
        TimelinePositionCursor(RightPixel(VideoClip(0, 4)) + 3);
        TimelineKeyPress('v');
        TimelineSelectClips({ VideoClip(0, 4) });
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        ASSERT(DetailsView(VideoClip(0, 4)).VCount(1));
        TimelinePositionCursor(RightPixel(VideoClip(0, 4)) + 12);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        ASSERT(DetailsView(VideoClip(0, 4)).VCount(2));
    }
    {
        StartTest("Cursor position maps onto key frames");
        TimelinePositionCursor(RightPixel(VideoClip(0, 4)) + 3);
        ASSERT(DetailsView(VideoClip(0, 4)).KeyFrameIndex(0));
        TimelinePositionCursor(RightPixel(VideoClip(0, 4)) + 12);
        ASSERT(DetailsView(VideoClip(0, 4)).KeyFrameIndex(1));
    }
    {
        StartTest("Change key frames");

        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(0).Position(wxPoint{ 0, 0 }));
        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(1).Position(wxPoint{ 0, 0 }));

        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFrameButton(0));
        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(0).Position(wxPoint{ 0, 0 }));
        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(1).Position(wxPoint{ 0, 0 }));
        ASSERT(DetailsView(VideoClip(0,4)).KeyFrameIndex(0).Position(wxPoint{0,0}));

        SetValue(DetailsClipView()->getPositionXSpin(), -128);
        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(0).Position(wxPoint{ -128, 0 }));
        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(1).Position(wxPoint{ 0, 0 }));
        ASSERT(DetailsView(VideoClip(0,4)).KeyFrameIndex(0).Position(wxPoint{ -128,0 }));

        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFrameButton(1));
        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(0).Position(wxPoint{ -128, 0 }));
        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(1).Position(wxPoint{ 0, 0 }));
        ASSERT(DetailsView(VideoClip(0,4)).KeyFrameIndex(1).Position(wxPoint{ 0,0 }));

        SetValue(DetailsClipView()->getPositionYSpin(), 128);
        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(0).Position(wxPoint{ -128, 0 }));
        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(1).Position(wxPoint{ 0, 128 }));
        ASSERT(DetailsView(VideoClip(0,4)).KeyFrameIndex(1).Position(wxPoint{ 0, 128 }));
    }
    {
        StartTest("Playback");
        TimelinePositionCursor(LeftPixel(VideoClip(0, 4)));
        Play(500);
    }
}

void TestVideoKeyFrames::testImage()
{
    StartTestSuite();
    TimelineZoomIn(8);
    ExtendSequenceWithStillImage(getSequence());
    pixel KeyFrame2Pixel = HCenter(VideoClip(0, 7));
    pixel KeyFrame1Pixel = KeyFrame2Pixel - 50;
    pixel KeyFrame3Pixel = KeyFrame2Pixel + 50;
    {
        StartTest("Create key frames");
        TimelineSelectClips({ VideoClip(0, 7) });
        TimelinePositionCursor(KeyFrame1Pixel);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        TimelinePositionCursor(KeyFrame2Pixel);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        TimelinePositionCursor(KeyFrame3Pixel);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        TimelineKeyPress('v');
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails);
        ASSERT_EQUALS(DetailsClipView()->getVideoKeyFrameButtonCount(), 3);
    }
    {
        StartTest("Change key frames");
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFrameButton(0));
        SetValue(DetailsClipView()->getRotationSpin(), -180);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFrameButton(1));
        SetValue(DetailsClipView()->getPositionXSlider(), -100);
        SetValue(DetailsClipView()->getPositionYSlider(), 100);
        SetValue(DetailsClipView()->getOpacitySlider(), 100);
        SetValue(DetailsClipView()->getScalingSpin(), 0.5);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFrameButton(2));
        SetValue(DetailsClipView()->getRotationSpin(), +180);
    }
    {
        StartTest("Playback");
        Play(KeyFrame1Pixel - 10, 3000);
    }
}

void TestVideoKeyFrames::testMovingKeyFramePosition()
{
    StartTestSuite();
    TimelineZoomIn(2);
    pixel KeyFrame2Pixel{ HCenter(VideoClip(0, 4)) };
    pixel KeyFrame1Pixel{ KeyFrame2Pixel - 50 };
    pixel KeyFrame3Pixel{ KeyFrame2Pixel + 50 };
    TimelineSelectClips({ VideoClip(0, 4) });
    TimelinePositionCursor(KeyFrame1Pixel);
    ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
    TimelinePositionCursor(KeyFrame2Pixel);
    ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
    TimelinePositionCursor(KeyFrame3Pixel);
    ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
    std::map<pts, model::KeyFramePtr> keyframes{ getVideoClip(VideoClip(0,4))->getKeyFramesOfPerceivedClip() };
    ASSERT_EQUALS(keyframes.size(), 3);
    std::map<pixel, pts> mapPixelToPts
    {
        { KeyFrame1Pixel, keyframes.begin()->first },
        { KeyFrame2Pixel, std::next(keyframes.begin())->first },
        { KeyFrame3Pixel, std::next(keyframes.begin(), 2)->first },
    };
    std::map<pixel, size_t> mapPixelToIndex
    {
        { KeyFrame1Pixel, 0 },
        { KeyFrame2Pixel, 1 },
        { KeyFrame3Pixel, 2 },
    };
    pixel y{ BottomPixel(VideoClip(0, 4)) - 2 };

    auto CHECK_MOVE = [&](pixel from, pixel distance, pts target)
    {
        size_t index{ mapPixelToIndex.find(from)->second };
        pts original{ mapPixelToPts.find(from)->second };
        TimelineMove(wxPoint(from, y));
        TimelineLeftDown();
        ASSERT(DetailsView(VideoClip(0, 4)).KeyFrameIndex(index));
        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(index).KeyFrameOffset(original));
        TimelineMove(wxPoint(from + distance, y));
        ASSERT(DetailsView(VideoClip(0, 4)).KeyFrameIndex(index));
        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(index).KeyFrameOffset(target));
        TimelineLeftUp();
        ASSERT(DetailsView(VideoClip(0, 4)).KeyFrameIndex(index));
        ASSERT(DetailsView(VideoClip(0, 4)).KeyFrameIndex(index).KeyFrameOffset(target));
        Undo();
        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(index).KeyFrameOffset(original));
        TimelineSelectClips({});
        ASSERT(DetailsView(nullptr));
    };
    {
        StartTest("Left key frame");
        CHECK_MOVE(KeyFrame1Pixel,  -10, mapPixelToPts[KeyFrame1Pixel] + getTimeline().getZoom().pixelsToPts(-10));
        CHECK_MOVE(KeyFrame1Pixel,  +10, mapPixelToPts[KeyFrame1Pixel] + getTimeline().getZoom().pixelsToPts(+10));
        CHECK_MOVE(KeyFrame1Pixel, -200, 0);
        CHECK_MOVE(KeyFrame1Pixel, +200, mapPixelToPts[KeyFrame2Pixel] - 1);
    }
    {
        StartTest("Middle key frame");
        CHECK_MOVE(KeyFrame2Pixel, -10, mapPixelToPts[KeyFrame2Pixel] + getTimeline().getZoom().pixelsToPts(-10));
        CHECK_MOVE(KeyFrame2Pixel, +10, mapPixelToPts[KeyFrame2Pixel] + getTimeline().getZoom().pixelsToPts(+10));
        CHECK_MOVE(KeyFrame2Pixel, -200, mapPixelToPts[KeyFrame1Pixel] + 1);
        CHECK_MOVE(KeyFrame2Pixel, +200, mapPixelToPts[KeyFrame3Pixel] - 1);
    }
    {
        StartTest("Right key frame");
        CHECK_MOVE(KeyFrame3Pixel, -10, mapPixelToPts[KeyFrame3Pixel] + getTimeline().getZoom().pixelsToPts(-10));
        CHECK_MOVE(KeyFrame3Pixel, +10, mapPixelToPts[KeyFrame3Pixel] + getTimeline().getZoom().pixelsToPts(+10));
        CHECK_MOVE(KeyFrame3Pixel, -200, mapPixelToPts[KeyFrame2Pixel] + 1);
        CHECK_MOVE(KeyFrame3Pixel, +200, VideoClip(0,4)->getPerceivedLength() - 1 + 1); // +1: right most key frame position is AFTER the last frame position
    }
}

void TestVideoKeyFrames::testEditingAfterMoving()
{
    StartTestSuite();
    TimelineZoomIn(2);
    pixel KeyFramePixel{ HCenter(VideoClip(0, 4)) };
    TimelineSelectClips({ VideoClip(0, 4) });
    TimelinePositionCursor(KeyFramePixel);
    ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
    pixel y{ BottomPixel(VideoClip(0, 4)) - 2 };
    TimelineMove(wxPoint(KeyFramePixel, y));
    TimelineLeftDown();
    ASSERT(DetailsView(VideoClip(0, 4)).KeyFrameIndex(0));
    ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(0));
    TimelineMove(wxPoint(KeyFramePixel + 56, y));
    ASSERT(DetailsView(VideoClip(0, 4)).KeyFrameIndex(0));
    ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(0));
    TimelineLeftUp();
    SetValue(DetailsClipView()->getRotationSpin(), 97.7);
    ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(0).Rotation(rational64{ 977,10 }));
}

void TestVideoKeyFrames::testAbortMovingKeyFrame()
{
    StartTestSuite();
    TimelineZoomIn(2);
    pixel KeyFramePixel{ HCenter(VideoClip(0, 4)) };
    TimelineSelectClips({ VideoClip(0, 4) });
    TimelinePositionCursor(KeyFramePixel);
    ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
    std::map<pts, model::KeyFramePtr> keyframes{ getVideoClip(VideoClip(0,4))->getKeyFramesOfPerceivedClip() };
    pts pos{ keyframes.begin()->first };
    ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(0).KeyFrameOffset(pos));
    pixel y{ BottomPixel(VideoClip(0, 4)) - 2 };
    TimelineMove(wxPoint(KeyFramePixel, y));
    TimelineLeftDown();
    ASSERT(DetailsView(VideoClip(0, 4)).KeyFrameIndex(0));
    ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(0));
    TimelineMove(wxPoint(KeyFramePixel + 56, y));
    ASSERT(DetailsView(VideoClip(0, 4)).KeyFrameIndex(0));
    ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(0));
    TimelineKeyPress(WXK_ESCAPE);
    TimelineLeftUp();
    ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(0).KeyFrameOffset(pos));
    ASSERT(DetailsView(VideoClip(0, 4)).VAdd(true));
    ASSERT(!DetailsClipView()->getRotationSlider()->IsEnabled()); // No key frame
}

} // namespace
