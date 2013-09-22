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

#include "TestDragAndDrop.h"

#include "CreateTransition.h"
#include "Cursor.h"
#include "Drag.h"
#include "HelperApplication.h"
#include "HelperProjectView.h"
#include "HelperTimeline.h"
#include "HelperTimelineAssert.h"
#include "HelperTimelineDrag.h"
#include "HelperTimelinesView.h"
#include "HelperTimelineTrim.h"
#include "HelperTransition.h"
#include "HelperWindow.h"
#include "Layout.h"
#include "Timeline.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestDragAndDrop::setUp()
{
    mProjectFixture.init();
}

void TestDragAndDrop::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASESMove(
//////////////////////////////////////////////////////////////////////////

void TestDragAndDrop::testSnapping()
{
    StartTestSuite();
    Zoom level(6);
    PositionCursor(LeftPixel(VideoClip(0,2)) + 10); // Just a bit to the right of the leftmost point of this clip
    ConfigFixture.SnapToClips(false).SnapToCursor(true);
    {
        StartTest("No snapping when dragged beyond snap distance");
        Drag(From(Center(VideoClip(0,0))).AlignLeft(CursorPosition() + gui::Layout::SnapDistance + 1).DontReleaseMouse());
        ASSERT_ZERO(getTimeline().getDrag().getSnapOffset());
        LeftUp();
        ASSERT_MORE_THAN(VideoClip(0,2)->getRightPts(), getTimeline().getCursor().getLogicalPosition());
        ASSERT_ZERO(getTimeline().getDrag().getSnapOffset());
        Undo();
    }
    {
        StartTest("Snap to cursor when inside snap distance");
        Drag(From(Center(VideoClip(0,0))).AlignLeft(CursorPosition() + gui::Layout::SnapDistance - 1).DontReleaseMouse());
        ASSERT_NONZERO(getTimeline().getDrag().getSnapOffset());
        LeftUp();
        ASSERT_EQUALS(VideoClip(0,2)->getRightPts(), getTimeline().getCursor().getLogicalPosition()); // due to snapping
        ASSERT_ZERO(getTimeline().getDrag().getSnapOffset()); // reset
        Undo();
    }
    {
        StartTest("Temporarily disable snapping");
        Drag(From(Center(VideoClip(0,0))).AlignLeft(CursorPosition() + gui::Layout::SnapDistance - 1).DontReleaseMouse());
        ASSERT_NONZERO(getTimeline().getDrag().getSnapOffset());
        Type('d'); // disable snapping
        ASSERT_ZERO(getTimeline().getDrag().getSnapOffset());
        Type('d'); // enable snapping
        ASSERT_NONZERO(getTimeline().getDrag().getSnapOffset());
        Type('d'); // disable snapping
        LeftUp();
        ASSERT_MORE_THAN(VideoClip(0,2)->getRightPts(), getTimeline().getCursor().getLogicalPosition());
        ASSERT_ZERO(getTimeline().getDrag().getSnapOffset());
        Undo();
    }

}

} // namespace