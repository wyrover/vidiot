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
        ASSERT(!DetailsClipView()->getVideoKeyFramesHomeButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesPrevButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesNextButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesEndButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesAddButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesRemoveButton()->IsEnabled());
        ASSERT_ZERO(DetailsClipView()->getVideoKeyFrameButtonCount());
    }
    {
        StartTest("Cursor after clip: All keys disabled.");
        TimelinePositionCursor(HCenter(VideoClip(0, 6)));
        ASSERT_ZERO(getVideoClip(VideoClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT(!DetailsClipView()->getVideoKeyFramesHomeButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesPrevButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesNextButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesEndButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesAddButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesRemoveButton()->IsEnabled());
        ASSERT_ZERO(DetailsClipView()->getVideoKeyFrameButtonCount());
    }
    {
        StartTest("Cursor inside clip: Add button enabled.");
        TimelinePositionCursor(HCenter(VideoClip(0, 4)));
        ASSERT_ZERO(getVideoClip(VideoClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT(!DetailsClipView()->getVideoKeyFramesHomeButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesPrevButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesNextButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesEndButton()->IsEnabled());
        ASSERT(DetailsClipView()->getVideoKeyFramesAddButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesRemoveButton()->IsEnabled());
        ASSERT_ZERO(DetailsClipView()->getVideoKeyFrameButtonCount());
    }
    {
        StartTest("Add first key frame.");
        TimelinePositionCursor(KeyFrame2Pixel);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        ASSERT_EQUALS(1, getVideoClip(VideoClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT(!DetailsClipView()->getVideoKeyFramesHomeButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesPrevButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesNextButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesEndButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesAddButton()->IsEnabled());
        ASSERT(DetailsClipView()->getVideoKeyFramesRemoveButton()->IsEnabled());
        ASSERT_EQUALS(1, DetailsClipView()->getVideoKeyFrameButtonCount());
        ASSERT_HISTORY_END(cmd::ProjectViewCreateSequence)(gui::timeline::cmd::EditClipDetails);
    }
    {
        StartTest("Position after first key frame.");
        TimelinePositionCursor(KeyFrame2Pixel + 50);
        ASSERT_EQUALS(1, getVideoClip(VideoClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT(DetailsClipView()->getVideoKeyFramesHomeButton()->IsEnabled());
        ASSERT(DetailsClipView()->getVideoKeyFramesPrevButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesNextButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesEndButton()->IsEnabled());
        ASSERT(DetailsClipView()->getVideoKeyFramesAddButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesRemoveButton()->IsEnabled());
        ASSERT_EQUALS(1, DetailsClipView()->getVideoKeyFrameButtonCount());
    }
    {
        StartTest("Position before first key frame.");
        TimelinePositionCursor(KeyFrame2Pixel - 50);
        ASSERT_EQUALS(1, getVideoClip(VideoClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT(!DetailsClipView()->getVideoKeyFramesHomeButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesPrevButton()->IsEnabled());
        ASSERT(DetailsClipView()->getVideoKeyFramesNextButton()->IsEnabled());
        ASSERT(DetailsClipView()->getVideoKeyFramesEndButton()->IsEnabled());
        ASSERT(DetailsClipView()->getVideoKeyFramesAddButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesRemoveButton()->IsEnabled());
        ASSERT_EQUALS(1, DetailsClipView()->getVideoKeyFrameButtonCount());
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
        StartTest("Home/Prev/Next/End button enabling disabling");
        TimelinePositionCursor(KeyFrame1Pixel - 10); // Before key frame 1
        ASSERT(!DetailsClipView()->getVideoKeyFramesHomeButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesPrevButton()->IsEnabled());
        ASSERT(DetailsClipView()->getVideoKeyFramesNextButton()->IsEnabled());
        ASSERT(DetailsClipView()->getVideoKeyFramesEndButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(0)->GetValue());
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(1)->GetValue());
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(2)->GetValue());
        TimelinePositionCursor(KeyFrame1Pixel); // On key frame 1
        ASSERT(!DetailsClipView()->getVideoKeyFramesHomeButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesPrevButton()->IsEnabled());
        ASSERT(DetailsClipView()->getVideoKeyFramesNextButton()->IsEnabled());
        ASSERT(DetailsClipView()->getVideoKeyFramesEndButton()->IsEnabled());
        ASSERT(DetailsClipView()->getVideoKeyFrameButton(0)->GetValue());
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(1)->GetValue());
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(2)->GetValue());
        TimelinePositionCursor(KeyFrame2Pixel - 40); // Between key frames 1 and 2
        ASSERT(DetailsClipView()->getVideoKeyFramesHomeButton()->IsEnabled());
        ASSERT(DetailsClipView()->getVideoKeyFramesPrevButton()->IsEnabled());
        ASSERT(DetailsClipView()->getVideoKeyFramesNextButton()->IsEnabled());
        ASSERT(DetailsClipView()->getVideoKeyFramesEndButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(0)->GetValue());
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(1)->GetValue());
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(2)->GetValue());
        TimelinePositionCursor(KeyFrame2Pixel); // On key frame 2
        ASSERT(DetailsClipView()->getVideoKeyFramesHomeButton()->IsEnabled());
        ASSERT(DetailsClipView()->getVideoKeyFramesPrevButton()->IsEnabled());
        ASSERT(DetailsClipView()->getVideoKeyFramesNextButton()->IsEnabled());
        ASSERT(DetailsClipView()->getVideoKeyFramesEndButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(0)->GetValue());
        ASSERT(DetailsClipView()->getVideoKeyFrameButton(1)->GetValue());
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(2)->GetValue());
        TimelinePositionCursor(KeyFrame2Pixel + 40); // Between key frames 2 and 3
        ASSERT(DetailsClipView()->getVideoKeyFramesHomeButton()->IsEnabled());
        ASSERT(DetailsClipView()->getVideoKeyFramesPrevButton()->IsEnabled());
        ASSERT(DetailsClipView()->getVideoKeyFramesNextButton()->IsEnabled());
        ASSERT(DetailsClipView()->getVideoKeyFramesEndButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(0)->GetValue());
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(1)->GetValue());
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(2)->GetValue());
        TimelinePositionCursor(KeyFrame3Pixel); // On key frame 3
        ASSERT(DetailsClipView()->getVideoKeyFramesHomeButton()->IsEnabled());
        ASSERT(DetailsClipView()->getVideoKeyFramesPrevButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesNextButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesEndButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(0)->GetValue());
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(1)->GetValue());
        ASSERT(DetailsClipView()->getVideoKeyFrameButton(2)->GetValue());
        TimelinePositionCursor(KeyFrame3Pixel + 10); // After key frame 3
        ASSERT(DetailsClipView()->getVideoKeyFramesHomeButton()->IsEnabled());
        ASSERT(DetailsClipView()->getVideoKeyFramesPrevButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesNextButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFramesEndButton()->IsEnabled());
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(0)->GetValue());
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(1)->GetValue());
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(2)->GetValue());
    }
    {
        StartTest("Use video key frames buttons and give frames specific parameters. Key frame 1: Rotation.");
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFrameButton(0));
        ASSERT(DetailsClipView()->getVideoKeyFrameButton(0)->GetValue());
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(1)->GetValue());
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(2)->GetValue());
        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(0).Opacity(255).Scaling(model::VideoScalingFitToFill).ScalingFactor(1).Alignment(model::VideoAlignmentCenter).Position(wxPoint{ 0,0 }).Rotation(0));
        ASSERT(DetailsView(VideoClip(0, 4)).Opacity(255).Scaling(model::VideoScalingFitToFill).ScalingFactor(1).Alignment(model::VideoAlignmentCenter).Position(wxPoint{ 0,0 }).Rotation(0));
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
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(0)->GetValue());
        ASSERT(DetailsClipView()->getVideoKeyFrameButton(1)->GetValue());
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(2)->GetValue());
        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(1).Opacity(255).Scaling(model::VideoScalingFitToFill).ScalingFactor(1).Alignment(model::VideoAlignmentCenter).Position(wxPoint{ 0,0 }).Rotation(0));
        ASSERT(DetailsView(VideoClip(0, 4)).Opacity(255).Scaling(model::VideoScalingFitToFill).ScalingFactor(1).Alignment(model::VideoAlignmentCenter).Position(wxPoint{ 0,0 }).Rotation(0));
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
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(0)->GetValue());
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(1)->GetValue());
        ASSERT(DetailsClipView()->getVideoKeyFrameButton(2)->GetValue());
        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(2).Opacity(255).Scaling(model::VideoScalingFitToFill).ScalingFactor(1).Alignment(model::VideoAlignmentCenter).Position(wxPoint{ 0,0 }).Rotation(0));
        ASSERT(DetailsView(VideoClip(0, 4)).Opacity(255).Scaling(model::VideoScalingFitToFill).ScalingFactor(1).Alignment(model::VideoAlignmentCenter).Position(wxPoint{ 0,0 }).Rotation(0));
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

    auto ASSERT_KEYFRAMES = [this](std::vector<pixel> positions)
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
        TimelineDeselectAllClips();
        ASSERT(DetailsView(nullptr));
    }
    {
        StartTest("Select first key frame");
        TimelineLeftClick(wxPoint(KeyFrame1Pixel, BottomPixel(VideoClip(0, 4)) - 2));
        ASSERT(DetailsView(VideoClip(0,4)).KeyFrameIndex(0));
        TimelineDeselectAllClips();
        ASSERT(DetailsView(nullptr));
    }
    {
        StartTest("Select middle key frame");
        TimelineLeftClick(wxPoint(KeyFrame2Pixel, BottomPixel(VideoClip(0, 4)) - 2));
        ASSERT(DetailsView(VideoClip(0, 4)).KeyFrameIndex(1));
        TimelineDeselectAllClips();
        ASSERT(DetailsView(nullptr));
    }
    {
        StartTest("Select last key frame");
        TimelineLeftClick(wxPoint(KeyFrame3Pixel, BottomPixel(VideoClip(0, 4)) - 2));
        ASSERT(DetailsView(VideoClip(0, 4)).KeyFrameIndex(2));
        TimelineDeselectAllClips();
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
        ASSERT_EQUALS(2, DetailsClipView()->getVideoKeyFrameButtonCount());
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
        ASSERT_EQUALS(2, DetailsClipView()->getVideoKeyFrameButtonCount());
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
        ASSERT_EQUALS(2, DetailsClipView()->getVideoKeyFrameButtonCount());
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
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(0)->GetValue());
        ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(10, 1));
        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(0).Rotation(90));
        TimelinePositionCursor(HCenter(VideoClip(0, 4)));
        ASSERT(DetailsClipView()->getVideoKeyFrameButton(0)->GetValue());
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipSpeed);
        Undo();
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails);
    }
    {
        StartTest("Decrease speed and verify key frame");
        SetValue(DetailsClipView()->getSpeedSlider(), 5000); // 5000 'to the left' sets speed to 0.5
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(0)->GetValue());
        ASSERT_CLIP_SPEED(VideoClip(0, 4), rational64(1, 2));
        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(0).Rotation(90));
        TimelinePositionCursor(HCenter(VideoClip(0, 4)));
        ASSERT(DetailsClipView()->getVideoKeyFrameButton(0)->GetValue());
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
        ASSERT_EQUALS(DetailsClipView()->getVideoKeyFrameButtonCount(), 2);
        ASSERT_HISTORY_END(gui::timeline::cmd::TrimClip);
    }
    {
        StartTest("Add frame by begin trim"); // todo in clip interval remove (during trimming) the key frames that are not visible anymore (otherwise, the clip starts with interpolated frames!)
        TimelineTrimLeft(VideoClip(0, 4), -100);
        // todo ASSERT_EQUALS(DetailsClipView()->getVideoKeyFrameButtonCount(), 2);
        // ASSERT_HISTORY_SKIP(5)(gui::timeline::cmd::TrimClip);
        ASSERT_EQUALS(DetailsClipView()->getVideoKeyFrameButtonCount(), 3);
        ASSERT_HISTORY_END(gui::timeline::cmd::TrimClip)(gui::timeline::cmd::TrimClip);
    }
    Undo(2);
    {
        StartTest("Remove frame by end trim");
        TimelineTrimRight(VideoClip(0, 4), -100);
        ASSERT_EQUALS(DetailsClipView()->getVideoKeyFrameButtonCount(), 2);
        ASSERT_HISTORY_END(gui::timeline::cmd::TrimClip);
    }
    {
        StartTest("Add frame by end trim"); // todo in clip interval remove (during trimming) the key frames that are not visible anymore (otherwise, the clip starts with interpolated frames!)
        TimelineTrimRight(VideoClip(0, 4), 100);
        // todo ASSERT_EQUALS(DetailsClipView()->getVideoKeyFrameButtonCount(), 2);
        // ASSERT_HISTORY_SKIP(5)(gui::timeline::cmd::TrimClip);
        ASSERT_EQUALS(DetailsClipView()->getVideoKeyFrameButtonCount(), 3);
        ASSERT_HISTORY_END(gui::timeline::cmd::TrimClip)(gui::timeline::cmd::TrimClip);
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
        ASSERT_EQUALS(DetailsClipView()->getVideoKeyFrameButtonCount(), 1);
        ASSERT(KeyFrame(VideoClip(0, 3)).KeyFrameIndex(0));
        ASSERT(DetailsView(VideoClip(0, 3)).KeyFrameIndex(0));
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesRemoveButton());
        ASSERT(KeyFrame(VideoClip(0, 3)));
        ASSERT(DetailsView(VideoClip(0, 3)));
        ASSERT_EQUALS(DetailsClipView()->getVideoKeyFrameButtonCount(), 0);
    }
    {
        StartTest("Zoom 8");
        TimelineZoomIn(6);
        TimelineSelectClips({ VideoClip(0,3) });
        TimelinePositionCursor(LeftPixel(VideoClip(0, 3)) + 20);
        TimelineKeyPress('v');
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        ASSERT_EQUALS(DetailsClipView()->getVideoKeyFrameButtonCount(), 1);
        ASSERT(KeyFrame(VideoClip(0, 3)).KeyFrameIndex(0));
        ASSERT(DetailsView(VideoClip(0, 3)).KeyFrameIndex(0));
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesRemoveButton());
        ASSERT(KeyFrame(VideoClip(0, 3)));
        ASSERT(DetailsView(VideoClip(0, 3)));
        ASSERT_EQUALS(DetailsClipView()->getVideoKeyFrameButtonCount(), 0);
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
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        ASSERT_EQUALS(DetailsClipView()->getVideoKeyFrameButtonCount(), 1);
        TimelinePositionCursor(LeftPixel(VideoClip(0, 5)) - 12);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        ASSERT_EQUALS(DetailsClipView()->getVideoKeyFrameButtonCount(), 2);
    }
    {
        StartTest("Cursor position maps onto key frames");
        TimelinePositionCursor(LeftPixel(VideoClip(0, 5)) - 3);
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(0)->GetValue());
        ASSERT(DetailsClipView()->getVideoKeyFrameButton(1)->GetValue());
        TimelinePositionCursor(LeftPixel(VideoClip(0, 5)) - 12);
        ASSERT(DetailsClipView()->getVideoKeyFrameButton(0)->GetValue());
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(1)->GetValue());
    }
    {
        StartTest("Change key frames");
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFrameButton(0));
        SetValue(DetailsClipView()->getPositionXSpin(), -128);
        ASSERT(KeyFrame(VideoClip(0, 5)).KeyFrameIndex(0).Position(wxPoint{ -128, 0 }));
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFrameButton(1));
        SetValue(DetailsClipView()->getPositionYSpin(), 128);
        ASSERT(KeyFrame(VideoClip(0, 5)).KeyFrameIndex(1).Position(wxPoint{ 0, 128 }));
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
        ASSERT_EQUALS(DetailsClipView()->getVideoKeyFrameButtonCount(), 1);
        TimelinePositionCursor(RightPixel(VideoClip(0, 4)) + 12);
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
        ASSERT_EQUALS(DetailsClipView()->getVideoKeyFrameButtonCount(), 2);
    }
    {
        StartTest("Cursor position maps onto key frames");
        TimelinePositionCursor(RightPixel(VideoClip(0, 4)) + 3);
        ASSERT(DetailsClipView()->getVideoKeyFrameButton(0)->GetValue());
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(1)->GetValue());
        TimelinePositionCursor(RightPixel(VideoClip(0, 4)) + 12);
        ASSERT(!DetailsClipView()->getVideoKeyFrameButton(0)->GetValue());
        ASSERT(DetailsClipView()->getVideoKeyFrameButton(1)->GetValue());
    }
    {
        StartTest("Change key frames");
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFrameButton(0));
        SetValue(DetailsClipView()->getPositionXSpin(), -128);
        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(0).Position(wxPoint{ -128, 0 }));
        ButtonTriggerPressed(DetailsClipView()->getVideoKeyFrameButton(1));
        SetValue(DetailsClipView()->getPositionYSpin(), 128);
        ASSERT(KeyFrame(VideoClip(0, 4)).KeyFrameIndex(1).Position(wxPoint{ 0, 128 }));
    }
    {
        StartTest("Playback");
        TimelinePositionCursor(LeftPixel(VideoClip(0, 5)));
        Play(500);
    }
}

} // namespace
