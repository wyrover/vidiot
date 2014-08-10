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

#include "TestUiTimelinePopupMenuMakeRoomForTransition.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestUiTimelinePopupMenuMakeRoomForTransition::setUp()
{
    mProjectFixture.init();
}

void TestUiTimelinePopupMenuMakeRoomForTransition::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestUiTimelinePopupMenuMakeRoomForTransition::testMakeRoomForCrossfade()
{
    StartTestSuite();
    TimelineZoomIn(6);
    ConfigFixture.SnapToClips(false);
    pts defaultTransitionLength = Config::ReadLong(Config::sPathDefaultTransitionLength);
    {
        StartTest("Left clip - small hidden extension, right clip - none: Crossfade only has half the default length.");
        TimelineShiftTrim(RightCenter(VideoClip(0,0)),RightCenter(VideoClip(0,0)) - wxPoint(getTimeline().getZoom().ptsToPixels(defaultTransitionLength / 2),0)); // Make left clip 'extendable' on its right side, with length 12
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0) - defaultTransitionLength / 2);
        pts OriginalLengthOfLeftClip = VideoClip(0,0)->getLength();
        TimelineOpenPopupMenuAt(Center(VideoClip(0,0)));
        TimelineKeyPress('n'); // Cross-fade to &next
        ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(      AudioClip      )(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), AudioClip(0,1)->getRightPts());
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), OriginalLengthOfLeftClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), defaultTransitionLength / 2);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1) - defaultTransitionLength / 2);
        Undo(2);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0)); // To check that the additional trimming is undone also
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
    }
    {
        StartTest("Left clip - large hidden extension, right clip - none: Crossfade is positioned entirely to the right of the cut.");
        TimelineShiftTrim(RightCenter(VideoClip(0,0)),Center(VideoClip(0,0))); // Make left clip 'extendable' on its right side
        pts OriginalLengthOfLeftClip = VideoClip(0,0)->getLength();
        TimelineOpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('p'); // Cross-fade from &previous
        ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(      AudioClip      )(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), AudioClip(0,1)->getRightPts());
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), OriginalLengthOfLeftClip);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), defaultTransitionLength);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1) - defaultTransitionLength);
        Undo(2);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0)); // To check that the additional trimming is undone also
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
    }
    {
        StartTest("Right clip - small 'hidden extension', left clip: none: Crossfade is positioned entirely to the left of the cut.");
        TimelineShiftTrim(LeftCenter(VideoClip(0,1)), LeftCenter(VideoClip(0,1)) + wxPoint(getTimeline().getZoom().ptsToPixels(defaultTransitionLength / 2),0)); // Make right clip 'extendable' on its left side, with length 12
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1) - defaultTransitionLength / 2);
        pts OriginalLengthOfRightClip = VideoClip(0,1)->getLength();
        TimelineOpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('p'); // Cross-fade from &previous
        ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip      )(      AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), AudioClip(0,1)->getRightPts());
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0) - defaultTransitionLength / 2);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), defaultTransitionLength / 2);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), OriginalLengthOfRightClip);
        Undo(2);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0)); // To check that the additional trimming is undone also
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
    }
    {
        StartTest("Right clip - large 'hidden extension', left clip: none: Crossfade is positioned entirely to the left of the cut.");
        TimelineShiftTrim(LeftCenter(VideoClip(0,1)),Center(VideoClip(0,1))); // Make right clip 'extendable' on its left side
        pts OriginalLengthOfRightClip = VideoClip(0,1)->getLength();
        TimelineOpenPopupMenuAt(Center(VideoClip(0,0)));
        TimelineKeyPress('n'); // Cross-fade to &next
        ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(AudioClip      )(      AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), AudioClip(0,1)->getRightPts());
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0) - defaultTransitionLength);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), defaultTransitionLength);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), OriginalLengthOfRightClip);
        Undo(2);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0)); // To check that the additional trimming is undone also
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
    }

    {
        StartTest("Left clip - small 'hidden extension', right clip - large one: Crossfade positioned to the left of the cut mostly.");
        TimelineShiftTrim(RightCenter(VideoClip(0,0)),RightCenter(VideoClip(0,0)) - wxPoint(getTimeline().getZoom().ptsToPixels(defaultTransitionLength / 4),0)); // Make left clip 'extendable' on its right side, with length 6
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0) - defaultTransitionLength / 4);
        TimelineShiftTrim(LeftCenter(VideoClip(0,1)),Center(VideoClip(0,1))); // Make right clip 'extendable' (a lot) on its left side
        pts OriginalLengthOfLeftClip = VideoClip(0,0)->getLength();
        pts OriginalLengthOfRightClip = VideoClip(0,1)->getLength();
        TimelineOpenPopupMenuAt(Center(VideoClip(0,0)));
        TimelineKeyPress('n'); // Cross-fade to &next
        ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(      AudioClip      )(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), AudioClip(0,1)->getRightPts());
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), OriginalLengthOfLeftClip - 3 * (defaultTransitionLength / 4) ); // 3/4 of the transition is 'to the left'
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), defaultTransitionLength);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), OriginalLengthOfRightClip - defaultTransitionLength / 4); // 1/4 of the transition is 'to the right'
        Undo(3);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0)); // To check that the additional trimming is undone also
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
    }
    {
        StartTest("Left clip has large 'hidden extension', right a small one: Crossfade positioned as much to the right of the cut as possible.");
        TimelineShiftTrim(RightCenter(VideoClip(0,0)),Center(VideoClip(0,0))); // Make left clip 'extendable' (a lot) on its right side
        TimelineShiftTrim(LeftCenter(VideoClip(0,1)), LeftCenter(VideoClip(0,1)) + wxPoint(getTimeline().getZoom().ptsToPixels(defaultTransitionLength / 4),0)); // Make right clip 'extendable' on its left side, with length 6
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1) - defaultTransitionLength / 4);
        pts OriginalLengthOfLeftClip = VideoClip(0,0)->getLength();
        pts OriginalLengthOfRightClip = VideoClip(0,1)->getLength();
        TimelineOpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('p'); // Cross-fade from &previous
        ASSERT_VIDEOTRACK0(VideoClip)(Transition)(VideoClip)(VideoClip);
        ASSERT_AUDIOTRACK0(      AudioClip      )(AudioClip)(AudioClip);
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), AudioClip(0,1)->getRightPts());
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), OriginalLengthOfLeftClip - defaultTransitionLength / 4); // 1/4 of the transition is 'to the left'
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), defaultTransitionLength);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), OriginalLengthOfRightClip - 3 * (defaultTransitionLength / 4) ); // 3/4 of the transition is 'to the right'
        Undo(3);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,0)); // To check that the additional trimming is undone also
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,2));
    }
    {
        StartTest("Left clip is not extendible, right is: Add crossfade to next clip."); // bug once
        TimelineOpenPopupMenuAt(Center(VideoClip(0,0)));
        TimelineKeyPress('i'); // fade &in
        ASSERT_VIDEOTRACK0(Transition)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), defaultTransitionLength / 2);
        TimelineShiftTrim(RightCenter(VideoClip(0,1)), Center(VideoClip(0,0)));
        ASSERT_ZERO(VideoClip(0,1)->getLength());
        TimelineOpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('n');
        ASSERT_VIDEOTRACK0(Transition)(VideoClip)(VideoClip)(VideoClip); // No transition added
        Undo(2);
    }
    {
        StartTest("Left clip is not extendible, right is: Add crossfade from previous clip."); // bug once
        TimelineOpenPopupMenuAt(Center(VideoClip(0,0)));
        TimelineKeyPress('i'); // fade &in
        ASSERT_VIDEOTRACK0(Transition)(VideoClip)(VideoClip)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), defaultTransitionLength / 2);
        TimelineShiftTrim(RightCenter(VideoClip(0,1)), Center(VideoClip(0,0)));
        ASSERT_ZERO(VideoClip(0,1)->getLength());
        TimelineOpenPopupMenuAt(Center(VideoClip(0,2)));
        TimelineKeyPress('p');
        ASSERT_VIDEOTRACK0(Transition)(VideoClip)(VideoClip)(VideoClip); // No transition added
        Undo(2);
    }
    {
        StartTest("Right clip is not extendible, left is: Add crossfade to next clip."); // bug once
        TimelineOpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('o'); // fade &out
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), defaultTransitionLength / 2);
        TimelineShiftTrim(LeftCenter(VideoClip(0,1)), Center(VideoClip(0,2)));
        ASSERT_ZERO(VideoClip(0,1)->getLength());
        TimelineOpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('n');
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        Undo(2);
    }
    {
        StartTest("Right clip is not extendible, left is: Add crossfade from previous clip."); // bug once
        TimelineOpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('o'); // fade &out
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), defaultTransitionLength / 2);
        TimelineShiftTrim(LeftCenter(VideoClip(0,1)), Center(VideoClip(0,2)));
        ASSERT_ZERO(VideoClip(0,1)->getLength());
        TimelineOpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('n');
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip);
        Undo(2);
    }
    {
        StartTest("Both clips not extendible: Add crossfade to next clip."); // bug once
        TimelineOpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('o'); // fade &out
        TimelineShiftTrim(LeftCenter(VideoClip(0,1)), RightCenter(VideoClip(0,1)) + wxPoint(10,0));
        TimelineOpenPopupMenuAt(Center(VideoClip(0,0)));
        TimelineKeyPress('i'); // fade &in
        TimelineShiftTrim(RightCenter(VideoClip(0,1)), LeftCenter(VideoClip(0,1)) - wxPoint(10,0));
        ASSERT_VIDEOTRACK0(Transition)(VideoClip)(VideoClip)(Transition);
        ASSERT_EQUALS(VideoClip(0,0)->getLength(), defaultTransitionLength / 2);
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), 0);
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), 0);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(), defaultTransitionLength / 2);
        TimelineOpenPopupMenuAt(Center(VideoClip(0,1)));
        TimelineKeyPress('n');
        ASSERT_VIDEOTRACK0(Transition)(VideoClip)(VideoClip)(Transition)(VideoClip); // No transition added
        StartTest("Both clips not extendible: Add crossfade from previous clip."); // bug once
        TimelineOpenPopupMenuAt(Center(VideoClip(0,2)));
        TimelineKeyPress('p');
        ASSERT_VIDEOTRACK0(Transition)(VideoClip)(VideoClip)(Transition)(VideoClip); // No transition added
    }
}

} // namespace