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

#include "TestIntervals.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestIntervals::setUp()
{
    mProjectFixture.init();
}

void TestIntervals::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestIntervals::testRemoveSelectedIntervals()
{
    StartTestSuite();
    ConfigFixture.SnapToClips(true);
    Zoom Level(2);

    StartTest("Make an interval from left to right and click 'delete all marked intervals'");
    ToggleInterval(HCenter(VideoClip(0,1)), HCenter(VideoClip(0,2)));
    TriggerMenu(ID_DELETEMARKED);
    ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0));
    ASSERT_LESS_THAN(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
    ASSERT_LESS_THAN(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
    pts video1Adjustedlength = VideoClip(0,1)->getLength();
    pts video2Adjustedlength = VideoClip(0,2)->getLength();
    ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
    Undo();
    Undo();
    StartTest("Make an interval from right to left and click 'delete all marked intervals'");
    ToggleInterval(HCenter(VideoClip(0,2)), HCenter(VideoClip(0,1)));
    TriggerMenu(ID_DELETEMARKED);
    ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0));
    ASSERT_EQUALS(VideoClip(0,1)->getLength(), video1Adjustedlength); // Verify there's no difference between selecting right-to-left and left-to-right
    ASSERT_EQUALS(VideoClip(0,2)->getLength(), video2Adjustedlength); // Verify there's no difference between selecting right-to-left and left-to-right
    ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
    Undo();
    Undo();
    StartTest("Make an interval that completely deletes a clip");
    ToggleInterval(LeftPixel(VideoClip(0,1)), RightPixel(VideoClip(0,1)));
    TriggerMenu(ID_DELETEMARKED);
    ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0));
    ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
    ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
    ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,4));
    Undo();
    Undo();
}

void TestIntervals::testRemoveUnselectedIntervals()
{
    StartTestSuite();
    ConfigFixture.SnapToClips(true);
    Zoom Level(2);

    StartTest("Make an interval from left to right and click 'delete all unmarked intervals'");
    ToggleInterval(10, HCenter(VideoClip(0,1))); // Can't use zero: the (test) code to trigger an interval first moves to the left a bit (and that would be < 0 in this case)
    ToggleInterval(HCenter(VideoClip(0,2)), RightPixel(VideoTrack(0)));
    TriggerMenu(ID_DELETEUNMARKED);
    ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0));
    ASSERT_LESS_THAN(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
    ASSERT_LESS_THAN(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
    pts video1Adjustedlength = VideoClip(0,1)->getLength();
    pts video2Adjustedlength = VideoClip(0,2)->getLength();
    ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
    Undo();
    Undo();
    Undo();
    StartTest("Make an interval from right to left and click 'delete all unmarked intervals'");
    ToggleInterval(RightPixel(VideoTrack(0)), HCenter(VideoClip(0,2)));
    ToggleInterval(HCenter(VideoClip(0,1)), 0);
    TriggerMenu(ID_DELETEUNMARKED);
    ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0));
    ASSERT_EQUALS(VideoClip(0,1)->getLength(), video1Adjustedlength); // Verify there's no difference between selecting right-to-left and left-to-right
    ASSERT_EQUALS(VideoClip(0,2)->getLength(), video2Adjustedlength); // Verify there's no difference between selecting right-to-left and left-to-right
    ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
    Undo();
    Undo();
    Undo();
    StartTest("Make an interval that completely deletes all but two clips");
    ToggleInterval(LeftPixel(VideoClip(0,1)), RightPixel(VideoClip(0,1)));
    ToggleInterval(LeftPixel(VideoClip(0,3)), RightPixel(VideoClip(0,3)));
    TriggerMenu(ID_DELETEUNMARKED);
    ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
    ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
    ASSERT_EQUALS(VideoTrack(0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1) + mProjectFixture.OriginalLengthOfVideoClip(0,3));
    Undo();
    Undo();
    Undo();
}

void TestIntervals::testRemoveEmptyIntervals()
{
    StartTestSuite();
    {
        StartTest("Remove all empty space between all clips.");
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip);
        TimelineLeftClick(Center(VideoClip(0,0)));
        TimelineKeyDown(wxMOD_CONTROL);
        TimelineLeftClick(Center(VideoClip(0,2)));
        TimelineLeftClick(Center(VideoClip(0,3)));
        TimelineLeftClick(Center(VideoClip(0,6)));
        TimelineKeyUp(wxMOD_CONTROL);
        TimelineKeyPress(WXK_DELETE);
        ASSERT_EQUALS(VideoTrack(0)->getClips().size(), 5);
        ASSERT_EQUALS(AudioTrack(0)->getClips().size(), 5);
        ASSERT_VIDEOTRACK0(EmptyClip)(VideoClip)(     EmptyClip      )(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(EmptyClip)(AudioClip)(     EmptyClip      )(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2) + mProjectFixture.OriginalLengthOfVideoClip(0,3));
        ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,4));
        ASSERT_EQUALS(VideoClip(0,4)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,5));
        ASSERT_EQUALS(AudioClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,0));
        ASSERT_EQUALS(AudioClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,1));
        ASSERT_EQUALS(AudioClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,2) + mProjectFixture.OriginalLengthOfAudioClip(0,3));
        ASSERT_EQUALS(AudioClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,4));
        ASSERT_EQUALS(AudioClip(0,4)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,5));
        TriggerMenu(ID_DELETEEMPTY);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoTrack(0)->getClips().size(), 3);
        ASSERT_EQUALS(AudioTrack(0)->getClips().size(), 3);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,4));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,5));
        ASSERT_EQUALS(AudioClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,1));
        ASSERT_EQUALS(AudioClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,4));
        ASSERT_EQUALS(AudioClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfAudioClip(0,5));
        Undo();
    }
}

void TestIntervals::testRemoveEmptyIntervalsWithOffset()
{
    StartTestSuite();
    Zoom level(2);
    ConfigFixture.SnapToClips(true);
    TriggerMenu(ID_ADDVIDEOTRACK);
    {
        StartTest("Remove empty intervals when clips are partially overlapping with the empty area");

        DragToTrack(1,VideoClip(0,3), model::IClipPtr());
        ASSERT_VIDEOTRACK1(EmptyClip)                      (VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(VideoClip           )(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip           )(AudioClip);

        Drag(From(Center(VideoClip(1,1))).AlignLeft(LeftPixel(VideoClip(0,4))));
        ASSERT_VIDEOTRACK1(EmptyClip)                                 (VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(           VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip)(AudioClip)(AudioClip);

        DragToTrack(1,VideoClip(0,4), model::IClipPtr());
        ASSERT_VIDEOTRACK1(EmptyClip)                                 (           VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(                      EmptyClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip)(AudioClip)(AudioClip);

        Drag(From(Center(VideoClip(1,1))).To(Center(VideoClip(1,1)) + wxPoint(-20,0)));
        ASSERT_VIDEOTRACK1(EmptyClip)                                 (           VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(                      EmptyClip           )(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip);

        Drag(From(Center(AudioClip(0,4))).To(RightCenter(AudioClip(0,2))));
        ASSERT_VIDEOTRACK1(EmptyClip)                                     (           VideoClip);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(                      EmptyClip           )(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip)(EmptyClip)(AudioClip)(EmptyClip)(AudioClip);

        pts gapLeft = VideoClip(1,1)->getLeftPts() - AudioClip(0,3)->getRightPts();
        pts gapRight = AudioClip(0,6)->getLength();
        pts expectedLength = VideoTrack(0)->getLength() - gapLeft - gapRight;

        TriggerMenu(ID_DELETEEMPTY);

        ASSERT_EQUALS(VideoTrack(0)->getLength(), expectedLength);
        ASSERT_EQUALS(AudioTrack(0)->getLength(), expectedLength);
        ASSERT_EQUALS(VideoTrack(1)->getLength(), VideoClip(0,4)->getLeftPts());
        Undo(6);
    }
}

} // namespace