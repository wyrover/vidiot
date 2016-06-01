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

#include "TestAudioKeyFrames.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestAudioKeyFrames::setUp()
{
    mProjectFixture.init();
}

void TestAudioKeyFrames::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestAudioKeyFrames::testAddKeyFrames()
{
    StartTestSuite();
    TimelineZoomIn(2);
    pixel KeyFrame2Pixel = HCenter(AudioClip(0, 4));
    pixel KeyFrame1Pixel = KeyFrame2Pixel - 50;
    pixel KeyFrame3Pixel = KeyFrame2Pixel + 50;
    TimelineSelectClips({ AudioClip(0,4) });
    {
        StartTest("Cursor before clip: All keys disabled.");
        TimelinePositionCursor(HCenter(AudioClip(0, 1)));
        ASSERT_ZERO(getAudioClip(AudioClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT(DetailsView(AudioClip(0, 4)).VHome(false).VPrev(false).VNext(false).VEnd(false).VAdd(false).VRemove(false).VCount(0));
    }
    {
        StartTest("Cursor after clip: All keys disabled.");
        TimelinePositionCursor(HCenter(AudioClip(0, 6)));
        ASSERT_ZERO(getAudioClip(AudioClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT(DetailsView(AudioClip(0, 4)).VHome(false).VPrev(false).VNext(false).VEnd(false).VAdd(false).VRemove(false).VCount(0));
    }
    {
        StartTest("Cursor inside clip: Add button enabled.");
        TimelinePositionCursor(HCenter(AudioClip(0, 4)));
        ASSERT_ZERO(getAudioClip(AudioClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT(DetailsView(AudioClip(0, 4)).VHome(false).VPrev(false).VNext(false).VEnd(false).VAdd(true).VRemove(false).VCount(0));
    }
    {
        StartTest("Add first key frame.");
        TimelinePositionCursor(KeyFrame2Pixel);
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
        ASSERT_EQUALS(1, getAudioClip(AudioClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT(DetailsView(AudioClip(0, 4)).VHome(false).VPrev(false).VNext(false).VEnd(false).VAdd(false).VRemove(true).VCount(1));
        ASSERT_HISTORY_END(cmd::ProjectViewCreateSequence)(gui::timeline::cmd::EditClipDetails);
    }
    {
        StartTest("Position after first key frame.");
        TimelinePositionCursor(KeyFrame2Pixel + 50);
        ASSERT_EQUALS(1, getAudioClip(AudioClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT(DetailsView(AudioClip(0, 4)).VHome(true).VPrev(true).VNext(false).VEnd(false).VAdd(true).VRemove(false).VCount(1));
    }
    {
        StartTest("Position before first key frame.");
        TimelinePositionCursor(KeyFrame2Pixel - 50);
        ASSERT_EQUALS(1, getAudioClip(AudioClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT(DetailsView(AudioClip(0, 4)).VHome(false).VPrev(false).VNext(true).VEnd(true).VAdd(true).VRemove(false).VCount(1));
    }
    {
        StartTest("Multiple key frames.");
        TimelinePositionCursor(KeyFrame1Pixel);
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails);
        TimelinePositionCursor(KeyFrame3Pixel);
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails);
        ASSERT_EQUALS(3, getAudioClip(AudioClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT_EQUALS(3, DetailsClipView()->getAudioKeyFrameButtonCount());
    }
    {
        StartTest("Use video key frames buttons and give frames specific parameters. Key frame 1.");
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFrameButton(0));
        ASSERT(KeyFrame(AudioClip(0, 4)).KeyFrameIndex(0).Volume(model::AudioKeyFrame::sVolumeDefault));
        ASSERT(DetailsView(AudioClip(0, 4)).KeyFrameIndex(0).Volume(model::AudioKeyFrame::sVolumeDefault));
        SetValue(DetailsClipView()->getVolumeSlider(), 120);
        ASSERT_HISTORY_END
            (gui::timeline::cmd::EditClipDetails) // Add key frame 1
            (gui::timeline::cmd::EditClipDetails) // Add key frame 2
            (gui::timeline::cmd::EditClipDetails) // Add key frame 3
            (gui::timeline::cmd::EditClipDetails);// Change volume
        ASSERT(KeyFrame(AudioClip(0, 4)).KeyFrameIndex(0).Volume(120));
        ASSERT(DetailsView(AudioClip(0, 4)).Volume(120));
    }
    {
        StartTest("Use video key frames buttons and give frames specific parameters. Key frame 2.");
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFrameButton(1));
        ASSERT(KeyFrame(AudioClip(0, 4)).KeyFrameIndex(1).Volume(model::AudioKeyFrame::sVolumeDefault));
        ASSERT(DetailsView(AudioClip(0, 4)).KeyFrameIndex(1).Volume(model::AudioKeyFrame::sVolumeDefault));
        SetValue(DetailsClipView()->getVolumeSlider(), model::AudioKeyFrame::sVolumeMax);
        ASSERT_HISTORY_END
            (gui::timeline::cmd::EditClipDetails) // Add key frame 1
            (gui::timeline::cmd::EditClipDetails) // Add key frame 2
            (gui::timeline::cmd::EditClipDetails) // Add key frame 3
            (gui::timeline::cmd::EditClipDetails) // Change volume for key frame 1
            (gui::timeline::cmd::EditClipDetails);// Change volume for key frame 2
        ASSERT(KeyFrame(AudioClip(0, 4)).KeyFrameIndex(1).Volume(model::AudioKeyFrame::sVolumeMax));
        ASSERT(DetailsView(AudioClip(0, 4)).Volume(model::AudioKeyFrame::sVolumeMax));

    }
    {
        StartTest("Use video key frames buttons and give frames specific parameters. Key frame 3.");
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFrameButton(2));
        ASSERT(KeyFrame(AudioClip(0, 4)).KeyFrameIndex(2).Volume(model::AudioKeyFrame::sVolumeDefault));
        ASSERT(DetailsView(AudioClip(0, 4)).KeyFrameIndex(2).Volume(model::AudioKeyFrame::sVolumeDefault));
        SetValue(DetailsClipView()->getVolumeSlider(), model::AudioKeyFrame::sVolumeMin);
        ASSERT_HISTORY_END
            (gui::timeline::cmd::EditClipDetails) // Add key frame 1
            (gui::timeline::cmd::EditClipDetails) // Add key frame 2
            (gui::timeline::cmd::EditClipDetails) // Add key frame 3
            (gui::timeline::cmd::EditClipDetails) // Change volume for key frame 1
            (gui::timeline::cmd::EditClipDetails) // Change volume for key frame 2
            (gui::timeline::cmd::EditClipDetails); // Change volume for key frame 3
        ASSERT(KeyFrame(AudioClip(0, 4)).KeyFrameIndex(2).Volume(model::AudioKeyFrame::sVolumeMin));
        ASSERT(DetailsView(AudioClip(0, 4)).Volume(model::AudioKeyFrame::sVolumeMin));
    }
    {
        StartTest("Interpolation before first key frame");
        TimelinePositionCursor(KeyFrame1Pixel - 20); // Before key frame 1
        ASSERT(DetailsView(AudioClip(0, 4)).Volume(120));
    }
    {
        StartTest("Interpolation between key frames");
        TimelinePositionCursor((KeyFrame1Pixel + KeyFrame2Pixel) / 2); // Between key frames 1 and 2
        ASSERT(DetailsView(AudioClip(0, 4)).Volume((120 + model::AudioKeyFrame::sVolumeMax) / 2));
        TimelinePositionCursor((KeyFrame2Pixel + KeyFrame3Pixel) / 2); // Between key frames 2 and 3
        ASSERT(DetailsView(AudioClip(0, 4)).Volume((model::AudioKeyFrame::sVolumeMin + model::AudioKeyFrame::sVolumeMax) / 2));
    }
    {
        StartTest("Interpolation after last key frame");
        TimelinePositionCursor(KeyFrame3Pixel + 10); // After key frame 3
        ASSERT(DetailsView(AudioClip(0, 4)).Volume(model::AudioKeyFrame::sVolumeMin));
    }
}

void TestAudioKeyFrames::testRemoveKeyFrames()
{
    StartTestSuite();
    TimelineZoomIn(2);
    pixel KeyFrame2Pixel{ HCenter(AudioClip(0, 4)) };
    pixel KeyFrame1Pixel{ KeyFrame2Pixel - 100 };
    pixel KeyFrame3Pixel{ KeyFrame2Pixel + 100 };
    TimelineSelectClips({ AudioClip(0,4) });

    auto ASSERT_KEYFRAMES = [&](std::vector<pixel> positions)
    {
        model::AudioClipPtr videoclip{ getAudioClip(AudioClip(0,4)) };
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
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
        TimelinePositionCursor(KeyFrame2Pixel);
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
        TimelinePositionCursor(KeyFrame3Pixel);
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
        ASSERT_EQUALS(3, getAudioClip(AudioClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT_EQUALS(3, DetailsClipView()->getAudioKeyFrameButtonCount());
        ASSERT_KEYFRAMES({ KeyFrame1Pixel, KeyFrame2Pixel, KeyFrame3Pixel });
    }
    {
        StartTest("Remove first key frame");
        TimelinePositionCursor(KeyFrame1Pixel);
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesRemoveButton());
        ASSERT_KEYFRAMES({ KeyFrame2Pixel, KeyFrame3Pixel });
        Undo();
        ASSERT_EQUALS(3, getAudioClip(AudioClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT_EQUALS(3, DetailsClipView()->getAudioKeyFrameButtonCount());
    }
    {
        StartTest("Remove middle key frame");
        TimelinePositionCursor(KeyFrame2Pixel);
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesRemoveButton());
        ASSERT_KEYFRAMES({ KeyFrame1Pixel, KeyFrame3Pixel });
        Undo();
        ASSERT_EQUALS(3, getAudioClip(AudioClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT_EQUALS(3, DetailsClipView()->getAudioKeyFrameButtonCount());
    }
    {
        StartTest("Remove last key frame");
        TimelinePositionCursor(KeyFrame3Pixel);
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesRemoveButton());
        ASSERT_KEYFRAMES({ KeyFrame1Pixel, KeyFrame2Pixel });
        Undo();
        ASSERT_EQUALS(3, getAudioClip(AudioClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT_EQUALS(3, DetailsClipView()->getAudioKeyFrameButtonCount());
    }
}

void TestAudioKeyFrames::testNavigation()
{
    StartTestSuite();
    TimelineZoomIn(2);
    pixel KeyFrame2Pixel{ HCenter(AudioClip(0, 4)) };
    pixel KeyFrame1Pixel{ KeyFrame2Pixel - 50 };
    pixel KeyFrame3Pixel{ KeyFrame2Pixel + 50 };
    pixel BeforeKeyFrame1{ KeyFrame1Pixel - 10 };
    pixel BetweenKeyFrame1AndKeyFrame2{ KeyFrame2Pixel - 40 };
    pixel BetweenKeyFrame2AndKeyFrame3{ KeyFrame3Pixel - 40 };
    pixel AfterKeyFrame3{ KeyFrame3Pixel + 10 };
    TimelineSelectClips({ AudioClip(0,4) });
    {
        StartTest("Prepare key frames.");
        TimelinePositionCursor(KeyFrame1Pixel);
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
        TimelinePositionCursor(KeyFrame2Pixel);
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
        TimelinePositionCursor(KeyFrame3Pixel);
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
        ASSERT_EQUALS(3, getAudioClip(AudioClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT(DetailsView(AudioClip(0, 4)).VCount(3));
        ASSERT_EQUALS(3, DetailsClipView()->getAudioKeyFrameButtonCount());
    }
    auto CHECK_BUTTON = [&](pixel from, wxButton* button, pixel to, size_t keyFrameIndex)
    {
        TimelinePositionCursor(from);
        ButtonTriggerPressed(button);
        ASSERT(DetailsView(AudioClip(0, 4)).KeyFrameIndex(keyFrameIndex));
    };
    {
        StartTest("Before first key frame");
        TimelinePositionCursor(BeforeKeyFrame1);
        ASSERT(DetailsView(AudioClip(0, 4)).VHome(false).VPrev(false).VNext(true).VEnd(true).VAdd(true).VRemove(false).VCount(3).NoKeyframe());
        CHECK_BUTTON(BeforeKeyFrame1, DetailsClipView()->getAudioKeyFramesNextButton(), KeyFrame1Pixel, 0);
        CHECK_BUTTON(BeforeKeyFrame1, DetailsClipView()->getAudioKeyFramesEndButton(), KeyFrame3Pixel, 2);
    }
    {
        StartTest("On first key frame");
        TimelinePositionCursor(KeyFrame1Pixel);
        ASSERT(DetailsView(AudioClip(0, 4)).VHome(false).VPrev(false).VNext(true).VEnd(true).VAdd(false).VRemove(true).VCount(3).KeyFrameIndex(0));
        CHECK_BUTTON(KeyFrame1Pixel, DetailsClipView()->getAudioKeyFramesNextButton(), KeyFrame2Pixel, 1);
        CHECK_BUTTON(KeyFrame1Pixel, DetailsClipView()->getAudioKeyFramesEndButton(), KeyFrame3Pixel, 2);
    }
    {
        StartTest("Between key frame 1 and key frame 2");
        TimelinePositionCursor(BetweenKeyFrame1AndKeyFrame2);
        ASSERT(DetailsView(AudioClip(0, 4)).VHome(true).VPrev(true).VNext(true).VEnd(true).VAdd(true).VRemove(false).VCount(3).NoKeyframe());
        CHECK_BUTTON(BetweenKeyFrame1AndKeyFrame2, DetailsClipView()->getAudioKeyFramesHomeButton(), KeyFrame1Pixel, 0);
        CHECK_BUTTON(BetweenKeyFrame1AndKeyFrame2, DetailsClipView()->getAudioKeyFramesPrevButton(), KeyFrame1Pixel, 0);
        CHECK_BUTTON(BetweenKeyFrame1AndKeyFrame2, DetailsClipView()->getAudioKeyFramesNextButton(), KeyFrame2Pixel, 1);
        CHECK_BUTTON(BetweenKeyFrame1AndKeyFrame2, DetailsClipView()->getAudioKeyFramesEndButton(), KeyFrame3Pixel, 2);
    }
    {
        StartTest("On key frame 2");
        TimelinePositionCursor(KeyFrame2Pixel);
        ASSERT(DetailsView(AudioClip(0, 4)).VHome(true).VPrev(true).VNext(true).VEnd(true).VAdd(false).VRemove(true).VCount(3).KeyFrameIndex(1));
        CHECK_BUTTON(KeyFrame2Pixel, DetailsClipView()->getAudioKeyFramesHomeButton(), KeyFrame1Pixel, 0);
        CHECK_BUTTON(KeyFrame2Pixel, DetailsClipView()->getAudioKeyFramesPrevButton(), KeyFrame1Pixel, 0);
        CHECK_BUTTON(KeyFrame2Pixel, DetailsClipView()->getAudioKeyFramesNextButton(), KeyFrame3Pixel, 2);
        CHECK_BUTTON(KeyFrame2Pixel, DetailsClipView()->getAudioKeyFramesEndButton(), KeyFrame3Pixel, 2);
    }
    {
        StartTest("Between key frame 2 and key frame 3");
        TimelinePositionCursor(BetweenKeyFrame2AndKeyFrame3);
        ASSERT(DetailsView(AudioClip(0, 4)).VHome(true).VPrev(true).VNext(true).VEnd(true).VAdd(true).VRemove(false).VCount(3).NoKeyframe());
        CHECK_BUTTON(BetweenKeyFrame2AndKeyFrame3, DetailsClipView()->getAudioKeyFramesHomeButton(), KeyFrame1Pixel, 0);
        CHECK_BUTTON(BetweenKeyFrame2AndKeyFrame3, DetailsClipView()->getAudioKeyFramesPrevButton(), KeyFrame2Pixel, 1);
        CHECK_BUTTON(BetweenKeyFrame2AndKeyFrame3, DetailsClipView()->getAudioKeyFramesNextButton(), KeyFrame3Pixel, 2);
        CHECK_BUTTON(BetweenKeyFrame2AndKeyFrame3, DetailsClipView()->getAudioKeyFramesEndButton(), KeyFrame3Pixel, 2);
    }
    {
        StartTest("On key frame 3");
        TimelinePositionCursor(KeyFrame3Pixel);
        ASSERT(DetailsView(AudioClip(0, 4)).VHome(true).VPrev(true).VNext(false).VEnd(false).VAdd(false).VRemove(true).VCount(3).KeyFrameIndex(2));
        CHECK_BUTTON(KeyFrame3Pixel, DetailsClipView()->getAudioKeyFramesHomeButton(), KeyFrame1Pixel, 0);
        CHECK_BUTTON(KeyFrame3Pixel, DetailsClipView()->getAudioKeyFramesPrevButton(), KeyFrame2Pixel, 1);
    }
    {
        StartTest("After key frame 3");
        TimelinePositionCursor(AfterKeyFrame3);
        ASSERT(DetailsView(AudioClip(0, 4)).VHome(true).VPrev(true).VNext(false).VEnd(false).VAdd(true).VRemove(false).VCount(3).NoKeyframe());
        CHECK_BUTTON(AfterKeyFrame3, DetailsClipView()->getAudioKeyFramesHomeButton(), KeyFrame1Pixel, 0);
        CHECK_BUTTON(AfterKeyFrame3, DetailsClipView()->getAudioKeyFramesPrevButton(), KeyFrame3Pixel, 2);
    }
}

void TestAudioKeyFrames::testSelectKeyFrameInTimeline()
{
    StartTestSuite();
    TimelineZoomIn(2);
    pixel KeyFrame2Pixel{ HCenter(AudioClip(0, 4)) };
    pixel KeyFrame1Pixel{ KeyFrame2Pixel - 100 };
    pixel KeyFrame3Pixel{ KeyFrame2Pixel + 100 };
    TimelineSelectClips({ AudioClip(0,4) });
    {
        StartTest("Prepare key frames.");
        TimelinePositionCursor(KeyFrame1Pixel);
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
        TimelinePositionCursor(KeyFrame2Pixel);
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
        TimelinePositionCursor(KeyFrame3Pixel);
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
        ASSERT_EQUALS(3, getAudioClip(AudioClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT_EQUALS(3, DetailsClipView()->getAudioKeyFrameButtonCount());
        TimelineSelectClips({});
        ASSERT(DetailsView(nullptr));
    }
    {
        StartTest("Select first key frame");
        TimelineLeftClick(wxPoint(KeyFrame1Pixel, BottomPixel(AudioClip(0, 4)) - 2));
        ASSERT(DetailsView(AudioClip(0,4)).KeyFrameIndex(0));
        TimelineSelectClips({});
        ASSERT(DetailsView(nullptr));
    }
    {
        StartTest("Select middle key frame");
        TimelineLeftClick(wxPoint(KeyFrame2Pixel, BottomPixel(AudioClip(0, 4)) - 2));
        ASSERT(DetailsView(AudioClip(0, 4)).KeyFrameIndex(1));
        TimelineSelectClips({});
        ASSERT(DetailsView(nullptr));
    }
    {
        StartTest("Select last key frame");
        TimelineLeftClick(wxPoint(KeyFrame3Pixel, BottomPixel(AudioClip(0, 4)) - 2));
        ASSERT(DetailsView(AudioClip(0, 4)).KeyFrameIndex(2));
        TimelineSelectClips({});
        ASSERT(DetailsView(nullptr));
    }
}

void TestAudioKeyFrames::testRemoveLastKeyFrame()
{
    StartTestSuite();
    TimelineZoomIn(2);

    ASSERT(DefaultKeyFrame(AudioClip(0, 4)).Volume(model::AudioKeyFrame::sVolumeDefault));
    TimelinePositionCursor(HCenter(AudioClip(0, 4)));
    ASSERT(DetailsView(nullptr));
    TimelineSelectClips({ AudioClip(0,4) });
    ASSERT(DetailsView(AudioClip(0, 4)));
    ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());

    StartTest("Change key frame.");
    ASSERT_EQUALS(1, getAudioClip(AudioClip(0, 4))->getKeyFramesOfPerceivedClip().size());
    SetValue(DetailsClipView()->getVolumeSlider(), 50);
    ASSERT(DefaultKeyFrame(AudioClip(0, 4)).Volume(model::AudioKeyFrame::sVolumeDefault));
    ASSERT(KeyFrame(AudioClip(0, 4)).KeyFrameIndex(0).Volume(50));

    StartTest("Delete key frame and verify default key frame updated");
    ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesRemoveButton());
    ASSERT(DefaultKeyFrame(AudioClip(0, 4)).Volume(50));
}

void TestAudioKeyFrames::testKeyFramesOnBeginAndEndOfClip()
{
    StartTestSuite();
    TimelineZoomIn(4);
    TimelineSelectClips({ AudioClip(0,1) });
    {
        StartTest("Prepare key frames.");
        TimelinePositionCursor(LeftPixel(AudioClip(0,1)));
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
        ASSERT(DetailsView(AudioClip(0, 1)).VCount(1));
        TimelinePositionCursor(LeftPixel(AudioClip(0, 2)));
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
        ASSERT(DetailsView(AudioClip(0, 1)).VCount(2));
        ASSERT(KeyFrame(AudioClip(0, 1)).KeyFrameIndex(0).KeyFrameOffset(0));
        ASSERT(KeyFrame(AudioClip(0, 1)).KeyFrameIndex(1).KeyFrameOffset(AudioClip(0,1)->getLength() - 1 + 1)); // +1: The rightmost key frame is AFTER the last key frame
    }
    {
        StartTest("Cursor after clip");
        TimelinePositionCursor(LeftPixel(AudioClip(0, 2)) + 5);
        ASSERT(DetailsView(AudioClip(0, 1)).VHome(true).VPrev(true).VNext(false).VEnd(false).VAdd(false).VRemove(true).VCount(2).KeyFrameIndex(1));
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesPrevButton());
        ASSERT(DetailsView(AudioClip(0, 1)).KeyFrameIndex(0));
    }
    {
        StartTest("Cursor before clip");
        TimelinePositionCursor(RightPixel(AudioClip(0, 0)) - 5);
        ASSERT(DetailsView(AudioClip(0, 1)).VHome(false).VPrev(false).VNext(true).VEnd(true).VAdd(false).VRemove(true).VCount(2).KeyFrameIndex(0));
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesNextButton());
        ASSERT(DetailsView(AudioClip(0, 1)).KeyFrameIndex(1));
    }
}

void TestAudioKeyFrames::testGetKeyFrameWithoutOffset()
{
    StartTestSuite();
    TimelineZoomIn(2);
    pixel InterpolatedPixel{ HCenter(AudioClip(0, 4)) };
    pixel KeyFrame1Pixel{ InterpolatedPixel - 50 };
    pixel KeyFrame2Pixel{ InterpolatedPixel + 50 };
    TimelineSelectClips({ AudioClip(0,4) });
    {
        StartTest("Prepare key frames.");
        TimelinePositionCursor(KeyFrame1Pixel);
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
        SetValue(DetailsClipView()->getVolumeSpin(), 60);
        TimelinePositionCursor(KeyFrame2Pixel);
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
        SetValue(DetailsClipView()->getVolumeSpin(), 80);
        ASSERT_EQUALS(2, getAudioClip(AudioClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT(DetailsView(AudioClip(0, 4)).VCount(2));
    }
    {
        StartTest("Get rotation for first key frame");
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFrameButton(0));
        ASSERT(DetailsView(AudioClip(0, 4)).KeyFrameIndex(0).Volume(60));
    }
    {
        StartTest("Get rotation before first key frame");
        TimelinePositionCursor(LeftPixel(AudioClip(0, 4)) + 5);
        ASSERT(DetailsView(AudioClip(0, 4)).Volume(60));
    }
    {
        StartTest("Get rotation for second key frame");
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFrameButton(1));
        ASSERT(DetailsView(AudioClip(0, 4)).KeyFrameIndex(1).Volume(80));
    }
    {
        StartTest("Get rotation after second key frame");
        TimelinePositionCursor(RightPixel(AudioClip(0, 4)) - 5);
        ASSERT(DetailsView(AudioClip(0, 4)).Volume(80));
    }
    {
        StartTest("Get rotation for interpolated frame");
        TimelinePositionCursor(HCenter(AudioClip(0, 4)));
        ASSERT(DetailsView(AudioClip(0, 4)).Volume(70));
    }
}

void TestAudioKeyFrames::testGetKeyFrameWithOffset()
{
    StartTestSuite();
    TimelineZoomIn(2);
    TimelineTrimLeft(AudioClip(0, 4), 50);
    pixel InterpolatedPixel{ HCenter(AudioClip(0, 4)) };
    pixel KeyFrame1Pixel{ InterpolatedPixel - 50 };
    pixel KeyFrame2Pixel{ InterpolatedPixel + 50 };
    TimelineSelectClips({ AudioClip(0,4) });
    {
        StartTest("Prepare key frames.");
        TimelinePositionCursor(KeyFrame1Pixel);
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
        SetValue(DetailsClipView()->getVolumeSpin(), 60);
        TimelinePositionCursor(KeyFrame2Pixel);
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
        SetValue(DetailsClipView()->getVolumeSpin(), 80);
        ASSERT_EQUALS(2, getAudioClip(AudioClip(0, 4))->getKeyFramesOfPerceivedClip().size());
        ASSERT(DetailsView(AudioClip(0, 4)).VCount(2));
    }
    {
        StartTest("Get rotation for first key frame");
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFrameButton(0));
        ASSERT(DetailsView(AudioClip(0, 4)).KeyFrameIndex(0).Volume(60));
    }
    {
        StartTest("Get rotation before first key frame");
        TimelinePositionCursor(LeftPixel(AudioClip(0, 4)) + 5);
        ASSERT(DetailsView(AudioClip(0, 4)).Volume(60));
    }
    {
        StartTest("Get rotation for second key frame");
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFrameButton(1));
        ASSERT(DetailsView(AudioClip(0, 4)).KeyFrameIndex(1).Volume(80));
    }
    {
        StartTest("Get rotation after second key frame");
        TimelinePositionCursor(RightPixel(AudioClip(0, 4)) - 5);
        ASSERT(DetailsView(AudioClip(0, 4)).Volume(80));
    }
    {
        StartTest("Get rotation for interpolated frame");
        TimelinePositionCursor(HCenter(AudioClip(0, 4)));
        ASSERT(DetailsView(AudioClip(0, 4)).Volume(70));
    }
}

void TestAudioKeyFrames::testGetKeyFrameWithTransition()
{
    StartTestSuite();
    TimelineZoomIn(2);
    MakeInOutTransitionAfterClip prepare(3, true);
    prepare.dontUndo();
    pixel InterpolatedPixel{ HCenter(AudioClip(0, 5)) };
    pixel KeyFrame1Pixel{ InterpolatedPixel - 20 };
    pixel KeyFrame2Pixel{ InterpolatedPixel + 20 };
    TimelineSelectClips({ AudioClip(0, 5) });
    {
        StartTest("Prepare key frames.");
        TimelinePositionCursor(KeyFrame1Pixel);
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
        SetValue(DetailsClipView()->getVolumeSpin(), 50);
        TimelinePositionCursor(KeyFrame2Pixel);
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
        SetValue(DetailsClipView()->getVolumeSpin(), 100);
        ASSERT_EQUALS(2, getAudioClip(AudioClip(0, 5))->getKeyFramesOfPerceivedClip().size());
        ASSERT(DetailsView(AudioClip(0, 5)).VCount(2));
    }
    {
        StartTest("Get volume for first key frame");
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFrameButton(0));
        ASSERT(DetailsView(AudioClip(0, 5)).KeyFrameIndex(0).Volume(50));
    }
    {
        StartTest("Get volume before first key frame");
        TimelinePositionCursor(LeftPixel(AudioClip(0, 4)) + 5);
        ASSERT(DetailsView(AudioClip(0, 5)).Volume(50));
    }
    {
        StartTest("Get volume for second key frame");
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFrameButton(1));
        ASSERT(DetailsView(AudioClip(0, 5)).KeyFrameIndex(1).Volume(100));
    }
    {
        StartTest("Get volume after second key frame");
        TimelinePositionCursor(RightPixel(AudioClip(0, 5)) - 5);
        ASSERT(DetailsView(AudioClip(0, 5)).Volume(100));
    }
    {
        StartTest("Get volume for interpolated frame");
        TimelinePositionCursor(HCenter(AudioClip(0, 5)));
        ASSERT(DetailsView(AudioClip(0, 5)).Volume(75));
    }
}

void TestAudioKeyFrames::testChangeClipSpeed()
{
    StartTestSuite();
    TimelineZoomIn(4); // Zooming in required to ensure that the last 'center positioning' is at exactly the key frame (on the clip with an increased speed)

    StartTest("Make key frame in center");
    Unlink(AudioClip(0, 4));
    TimelineDeleteClip(VideoClip(0, 4));
    TimelinePositionCursor(HCenter(AudioClip(0, 4)));
    TimelineSelectClips({ AudioClip(0,4) });
    ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
    SetValue(DetailsClipView()->getVolumeSlider(), 90);
    ASSERT_HISTORY_END
        (cmd::ProjectViewCreateAutoFolder)
        (cmd::ProjectViewCreateSequence)
        (gui::timeline::cmd::UnlinkClips)
        (gui::timeline::cmd::DeleteSelectedClips)
        (gui::timeline::cmd::EditClipDetails) // Add key frame
        (gui::timeline::cmd::EditClipDetails); // Volume change
    {
        StartTest("Increase speed and verify key frame");
        SetValue(DetailsClipView()->getSpeedSpin(), 10.0);
        ASSERT(DetailsView(AudioClip(0, 4)).NoKeyframe());
        ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(10, 1));
        ASSERT(KeyFrame(AudioClip(0, 4)).KeyFrameIndex(0).Volume(90));
        TimelinePositionCursor(HCenter(AudioClip(0, 4)));
        ASSERT(DetailsView(AudioClip(0, 4)).KeyFrameIndex(0));
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipSpeed);
        Undo();
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails);
    }
    {
        StartTest("Decrease speed and verify key frame");
        SetValue(DetailsClipView()->getSpeedSlider(), 5000); // 5000 'to the left' sets speed to 0.5
        ASSERT(DetailsView(AudioClip(0, 4)).NoKeyframe());
        ASSERT_CLIP_SPEED(AudioClip(0, 4), rational64(1, 2));
        ASSERT(KeyFrame(AudioClip(0, 4)).KeyFrameIndex(0).Volume(90));
        TimelinePositionCursor(HCenter(AudioClip(0, 4)));
        ASSERT(DetailsView(AudioClip(0, 4)).KeyFrameIndex(0));
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipSpeed);
        Undo();
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails);
    }
}

void TestAudioKeyFrames::testTrimAwayKeyFrames()
{
    StartTestSuite();
    TimelineZoomIn(2);
    pixel KeyFrame2Pixel = HCenter(AudioClip(0, 4));
    pixel KeyFrame1Pixel = KeyFrame2Pixel - 50;
    pixel KeyFrame3Pixel = KeyFrame2Pixel + 50;
    TimelineSelectClips({ AudioClip(0, 4) });
    TimelinePositionCursor(KeyFrame1Pixel);
    ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
    TimelinePositionCursor(KeyFrame2Pixel);
    ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
    TimelinePositionCursor(KeyFrame3Pixel);
    ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
    ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails);
    ASSERT_EQUALS(DetailsClipView()->getAudioKeyFrameButtonCount(), 3);
    {
        StartTest("Remove frame by begin trim");
        TimelineTrimLeft(AudioClip(0, 4), 100);
        ASSERT(DetailsView(AudioClip(0, 4)).VCount(2));
        ASSERT_HISTORY_END(gui::timeline::cmd::TrimClip);
    }
    {
        StartTest("Add frame by begin trim (impossible)");
        TimelineTrimLeft(AudioClip(0, 4), -100);
        ASSERT(DetailsView(AudioClip(0, 4)).VCount(2));
        ASSERT_HISTORY_END(gui::timeline::cmd::TrimClip)(gui::timeline::cmd::TrimClip);
    }
    {
        StartTest("Add frame again by Undoing");
        Undo(2);
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails);
        ASSERT(DetailsView(AudioClip(0, 4)).VCount(3));
    }
    {
        StartTest("Remove frame by end trim");
        TimelineTrimRight(AudioClip(0, 4), -100);
        ASSERT(DetailsView(AudioClip(0, 4)).VCount(2));
        ASSERT_HISTORY_END(gui::timeline::cmd::TrimClip);
    }
    {
        StartTest("Add frame by end trim (impossible)");
        TimelineTrimRight(AudioClip(0, 4), 100);
        ASSERT(DetailsView(AudioClip(0, 4)).VCount(2));
        ASSERT_HISTORY_END(gui::timeline::cmd::TrimClip)(gui::timeline::cmd::TrimClip);
    }
    {
        StartTest("Add frame again by Undoing");
        Undo(2);
        ASSERT_HISTORY_END(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails)(gui::timeline::cmd::EditClipDetails);
        ASSERT(DetailsView(AudioClip(0, 4)).VCount(3));
    }
}

void TestAudioKeyFrames::testBeginOffset()
{
    StartTestSuite();
    TimelineTrimLeft(AudioClip(0, 3), 100);
    {
        StartTest("Zoom 2");
        TimelineZoomIn(2);
        TimelineSelectClips({ AudioClip(0,3) });
        TimelinePositionCursor(LeftPixel(AudioClip(0, 3)) + 50);
        TimelineKeyPress('v');
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
        ASSERT(KeyFrame(AudioClip(0, 3)).KeyFrameIndex(0));
        ASSERT(DetailsView(AudioClip(0, 3)).VCount(1).KeyFrameIndex(0));
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesRemoveButton());
        ASSERT(KeyFrame(AudioClip(0, 3)));
        ASSERT(DetailsView(AudioClip(0, 3)).VCount(0).NoKeyframe());
    }
    {
        StartTest("Zoom 8");
        TimelineZoomIn(6);
        TimelineSelectClips({ AudioClip(0,3) });
        TimelinePositionCursor(LeftPixel(AudioClip(0, 3)) + 20);
        TimelineKeyPress('v');
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
        ASSERT(KeyFrame(AudioClip(0, 3)).KeyFrameIndex(0));
        ASSERT(DetailsView(AudioClip(0, 3)).VCount(1).KeyFrameIndex(0));
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesRemoveButton());
        ASSERT(KeyFrame(AudioClip(0, 3)));
        ASSERT(DetailsView(AudioClip(0, 3)).NoKeyframe().VCount(0));
    }
}

void TestAudioKeyFrames::testInTransition()
{
    StartTestSuite();
    TimelineZoomIn(8);
    MakeInOutTransitionAfterClip prepare(3, true);
    prepare.dontUndo();
    {
        StartTest("Make two key frames under an in transition");
        TimelinePositionCursor(LeftPixel(AudioClip(0, 5)) - 3);
        TimelineKeyPress('v');
        TimelineSelectClips({ AudioClip(0, 5) });
        ASSERT(DetailsView(AudioClip(0,5)).VAdd(true));
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
        ASSERT(DetailsView(AudioClip(0,5)).VAdd(false).VCount(1));
        TimelinePositionCursor(LeftPixel(AudioClip(0, 5)) - 12);
        ASSERT(DetailsView(AudioClip(0,5)).VAdd(true).VCount(1));
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
        ASSERT(DetailsView(AudioClip(0,5)).VAdd(false).VCount(2));
    }
    {
        StartTest("Cursor position maps onto key frames");
        TimelinePositionCursor(LeftPixel(AudioClip(0, 5)) - 3);
        ASSERT(DetailsView(AudioClip(0,5)).KeyFrameIndex(1));
        TimelinePositionCursor(LeftPixel(AudioClip(0, 5)) - 12);
        ASSERT(DetailsView(AudioClip(0,5)).KeyFrameIndex(0));
    }
    {
        StartTest("Change key frames");
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFrameButton(0));
        SetValue(DetailsClipView()->getVolumeSpin(), 20);
        ASSERT(KeyFrame(AudioClip(0, 5)).KeyFrameIndex(0).Volume(20));
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFrameButton(1));
        SetValue(DetailsClipView()->getVolumeSpin(), 120);
        ASSERT(KeyFrame(AudioClip(0, 5)).KeyFrameIndex(1).Volume(120));
    }
    {
        StartTest("Playback");
        TimelinePositionCursor(HCenter(AudioClip(0, 4)));
        Play(500);
    }
}

void TestAudioKeyFrames::testOutTransition()
{
    StartTestSuite();
    TimelineZoomIn(8);
    MakeInOutTransitionAfterClip prepare(4, true);
    prepare.dontUndo();
    {
        StartTest("Make two key frames under an out transition");
        TimelinePositionCursor(RightPixel(AudioClip(0, 4)) + 3);
        TimelineKeyPress('v');
        TimelineSelectClips({ AudioClip(0, 4) });
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
        ASSERT(DetailsView(AudioClip(0, 4)).VCount(1));
        TimelinePositionCursor(RightPixel(AudioClip(0, 4)) + 12);
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
        ASSERT(DetailsView(AudioClip(0, 4)).VCount(2));
    }
    {
        StartTest("Cursor position maps onto key frames");
        TimelinePositionCursor(RightPixel(AudioClip(0, 4)) + 3);
        ASSERT(DetailsView(AudioClip(0, 4)).KeyFrameIndex(0));
        TimelinePositionCursor(RightPixel(AudioClip(0, 4)) + 12);
        ASSERT(DetailsView(AudioClip(0, 4)).KeyFrameIndex(1));
    }
    {
        StartTest("Change key frames");
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFrameButton(0));
        SetValue(DetailsClipView()->getVolumeSpin(), 20);
        ASSERT(KeyFrame(AudioClip(0, 4)).KeyFrameIndex(0).Volume(20));
        ButtonTriggerPressed(DetailsClipView()->getAudioKeyFrameButton(1));
        SetValue(DetailsClipView()->getVolumeSpin(), 128);
        ASSERT(KeyFrame(AudioClip(0, 4)).KeyFrameIndex(1).Volume(128));
    }
    {
        StartTest("Playback");
        TimelinePositionCursor(LeftPixel(AudioClip(0, 5)));
        Play(500);
    }
}

void TestAudioKeyFrames::testMovingKeyFramePosition()
{
    StartTestSuite();
    TimelineZoomIn(2);
    pixel KeyFrame2Pixel{ HCenter(AudioClip(0, 4)) };
    pixel KeyFrame1Pixel{ KeyFrame2Pixel - 50 };
    pixel KeyFrame3Pixel{ KeyFrame2Pixel + 50 };
    TimelineSelectClips({ AudioClip(0, 4) });
    TimelinePositionCursor(KeyFrame1Pixel);
    ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
    TimelinePositionCursor(KeyFrame2Pixel);
    ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
    TimelinePositionCursor(KeyFrame3Pixel);
    ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
    std::map<pts, model::KeyFramePtr> keyframes{ getAudioClip(AudioClip(0,4))->getKeyFramesOfPerceivedClip() };
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
    pixel y{ BottomPixel(AudioClip(0, 4)) - 2 };

    auto CHECK_MOVE = [&](pixel from, pixel distance, pts target)
    {
        size_t index{ mapPixelToIndex.find(from)->second };
        pts original{ mapPixelToPts.find(from)->second };
        TimelineMove(wxPoint(from, y));
        TimelineLeftDown();
        ASSERT(DetailsView(AudioClip(0, 4)).KeyFrameIndex(index));
        ASSERT(KeyFrame(AudioClip(0, 4)).KeyFrameIndex(index).KeyFrameOffset(original));
        TimelineMove(wxPoint(from + distance, y));
        ASSERT(DetailsView(AudioClip(0, 4)).KeyFrameIndex(index));
        ASSERT(KeyFrame(AudioClip(0, 4)).KeyFrameIndex(index).KeyFrameOffset(target));
        TimelineLeftUp();
        ASSERT(DetailsView(AudioClip(0, 4)).KeyFrameIndex(index));
        ASSERT(DetailsView(AudioClip(0, 4)).KeyFrameIndex(index).KeyFrameOffset(target));
        Undo();
        ASSERT(KeyFrame(AudioClip(0, 4)).KeyFrameIndex(index).KeyFrameOffset(original));
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
        CHECK_MOVE(KeyFrame3Pixel, +200, AudioClip(0,4)->getPerceivedLength() - 1 + 1); // +1: right most key frame position is AFTER the last frame position
    }
}

void TestAudioKeyFrames::testEditingAfterMoving()
{
    StartTestSuite();
    TimelineZoomIn(2);
    pixel KeyFramePixel{ HCenter(AudioClip(0, 4)) };
    TimelineSelectClips({ AudioClip(0, 4) });
    TimelinePositionCursor(KeyFramePixel);
    ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
    pixel y{ BottomPixel(AudioClip(0, 4)) - 2 };
    TimelineMove(wxPoint(KeyFramePixel, y));
    TimelineLeftDown();
    ASSERT(DetailsView(AudioClip(0, 4)).KeyFrameIndex(0));
    ASSERT(KeyFrame(AudioClip(0, 4)).KeyFrameIndex(0));
    TimelineMove(wxPoint(KeyFramePixel + 56, y));
    ASSERT(DetailsView(AudioClip(0, 4)).KeyFrameIndex(0));
    ASSERT(KeyFrame(AudioClip(0, 4)).KeyFrameIndex(0));
    TimelineLeftUp();
    SetValue(DetailsClipView()->getVolumeSpin(), 97);
    ASSERT(KeyFrame(AudioClip(0, 4)).KeyFrameIndex(0).Volume(97));
}

void TestAudioKeyFrames::testAbortMovingKeyFrame()
{
    StartTestSuite();
    TimelineZoomIn(2);
    pixel KeyFramePixel{ HCenter(AudioClip(0, 4)) };
    TimelineSelectClips({ AudioClip(0, 4) });
    TimelinePositionCursor(KeyFramePixel);
    ButtonTriggerPressed(DetailsClipView()->getAudioKeyFramesAddButton());
    std::map<pts, model::KeyFramePtr> keyframes{ getAudioClip(AudioClip(0,4))->getKeyFramesOfPerceivedClip() };
    pts pos{ keyframes.begin()->first };
    ASSERT(KeyFrame(AudioClip(0, 4)).KeyFrameIndex(0).KeyFrameOffset(pos));
    pixel y{ BottomPixel(AudioClip(0, 4)) - 2 };
    TimelineMove(wxPoint(KeyFramePixel, y));
    TimelineLeftDown();
    ASSERT(DetailsView(AudioClip(0, 4)).KeyFrameIndex(0));
    ASSERT(KeyFrame(AudioClip(0, 4)).KeyFrameIndex(0));
    TimelineMove(wxPoint(KeyFramePixel + 56, y));
    ASSERT(DetailsView(AudioClip(0, 4)).KeyFrameIndex(0));
    ASSERT(KeyFrame(AudioClip(0, 4)).KeyFrameIndex(0));
    TimelineKeyPress(WXK_ESCAPE);
    TimelineLeftUp();
    ASSERT(KeyFrame(AudioClip(0, 4)).KeyFrameIndex(0).KeyFrameOffset(pos));
    ASSERT(DetailsView(AudioClip(0, 4)).VAdd(true));
    ASSERT(!DetailsClipView()->getVolumeSlider()->IsEnabled()); // No key frame
}

} // namespace
