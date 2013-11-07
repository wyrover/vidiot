// Copyright 2013 Eric Raijmakers.
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

#include "TestTrimming.h"

#include "AudioClip.h"
#include "AudioTrack.h"
#include "CreateTransition.h"
#include "Cursor.h"
#include "HelperApplication.h"
#include "HelperProjectView.h"
#include "HelperTimeline.h"
#include "HelperTimelineAssert.h"
#include "HelperTimelineDrag.h"
#include "HelperTimelinesView.h"
#include "HelperTimelineTrim.h"
#include "HelperTransition.h"
#include "HelperWindow.h"
#include "Timeline.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestTrimming::setUp()
{
    mProjectFixture.init();
}

void TestTrimming::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASESMove(
//////////////////////////////////////////////////////////////////////////

void TestTrimming::testSnapping()
{
    StartTestSuite();
    Zoom level(6);
    ConfigFixture.SnapToClips(true).SnapToCursor(true);
    PositionCursor(HCenter(VideoClip(0,2)));
    {
        StartTest("No snapping when dragged beyond snap distance");
        Trim(LeftCenter(VideoClip(0,2)),Center(VideoClip(0,2)) + wxPoint(gui::Layout::SnapDistance + 1,0),false);
        ASSERT_MORE_THAN(VideoClip(0,2)->getRightPts(), getTimeline().getCursor().getLogicalPosition());
        Undo();
    }
    {
        StartTest("Snap to cursor when inside snap distance");
        Trim(LeftCenter(VideoClip(0,2)),Center(VideoClip(0,2)) + wxPoint(gui::Layout::SnapDistance - 1,0),false);
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), getTimeline().getCursor().getLogicalPosition()); // due to snapping
        Undo();
    }
    {
        StartTest("Temporarily disable snapping");
        Trim(LeftCenter(VideoClip(0,2)),Center(VideoClip(0,2)) + wxPoint(gui::Layout::SnapDistance - 1,0),false,false);
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), getTimeline().getCursor().getLogicalPosition()); // due to snapping
        Type('d'); // disable snapping
        ASSERT_MORE_THAN(VideoClip(0,2)->getRightPts(), getTimeline().getCursor().getLogicalPosition());
        Type('d'); // enable snapping
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), getTimeline().getCursor().getLogicalPosition()); // due to snapping
        EndTrim(false);
        Undo();
    }

}

void TestTrimming::testKeyboardTrimming()
{
    StartTestSuite();
    Zoom level(3);
    {
        StartTest("Begin trim");
        PositionCursor(HCenter(VideoClip(0,2)));
        pts newlength = VideoClip(0,2)->getRightPts() - getTimeline().getCursor().getLogicalPosition();
        Type('b');
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), newlength);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
        ASSERT_EQUALS(getTimeline().getCursor().getLogicalPosition(), VideoClip(0,2)->getLeftPts());
        Undo();
    }
    {
        StartTest("End trim");
        PositionCursor(HCenter(VideoClip(0,2)));
        pts newlength = getTimeline().getCursor().getLogicalPosition() - VideoClip(0,2)->getLeftPts();
        Type('e');
        ASSERT_EQUALS(VideoClip(0,1)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,1));
        ASSERT_EQUALS(VideoClip(0,2)->getLength(), newlength);
        ASSERT_EQUALS(VideoClip(0,3)->getLength(), mProjectFixture.OriginalLengthOfVideoClip(0,3));
        ASSERT_EQUALS(getTimeline().getCursor().getLogicalPosition(), VideoClip(0,2)->getRightPts());
        Undo();
    }
    {
        StartTest("No change when cursor is on cut");
        ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();
        PositionCursor(LeftPixel(VideoClip(0,2)));
        Type('b');
        Type('e');
        ASSERT_CURRENT_COMMAND_TYPE<command::ProjectViewCreateSequence>();
    }
}

} // namespace