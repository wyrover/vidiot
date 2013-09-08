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

#include "TestScrolling.h"

#include "HelperTimeline.h"
#include "HelperTimelineDrag.h"
#include "HelperTimelinesView.h"
#include "Layout.h"
#include "Scrolling.h"
#include "SequenceView.h"
#include "Timeline.h"
#include "UtilLogWxwidgets.h"
#include "VideoTrack.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestScrolling::setUp()
{
    mProjectFixture.init();
}

void TestScrolling::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestScrolling::testScrollbarRepositioningAfterChangingZoom()
{
    StartTestSuite();
    Zoom Level(6); // Maximum
    ASSERT_EQUALS(getTimeline().getZoom().getCurrent(), rational(1,1));

    StartTest("Scroll using right down scrolling.");
    pixel from = LeftPixel(VideoClip(0,4));
    pixel to = HCenter(VideoClip(0,1));
    Move(wxPoint(from, gui::Layout::TimeScaleHeight));
    RightDown();
    Move(wxPoint(to, gui::Layout::TimeScaleHeight));
    RightUp();
    ASSERT_ZERO(getTimeline().getScrolling().getOffset().y);
    ASSERT_EQUALS(getTimeline().getScrolling().getOffset().x, from - to);

    pts center = getTimeline().getScrolling().getCenterPts();

    StartTest("Center pts position is kept aligned when zooming out via the keyboard.");
    Type('-');
    ASSERT_EQUALS(getTimeline().getZoom().getCurrent(), rational(1,2));
    ASSERT_EQUALS(getTimeline().getScrolling().getCenterPts(), center);

    StartTest("Center pts position is kept aligned when zooming in via the keyboard.");
    Type('=');
    ASSERT_EQUALS(getTimeline().getZoom().getCurrent(), rational(1,1));
    ASSERT_EQUALS(getTimeline().getScrolling().getCenterPts(), center);

    StartTest("Center pts position is kept aligned when zooming in via the wheel.");
    ControlDown();
    TimelineTriggerWheel(-1);
    ControlUp();
    ASSERT_EQUALS(getTimeline().getZoom().getCurrent(), rational(1,2));
    ASSERT_EQUALS(getTimeline().getScrolling().getCenterPts(), center);

    StartTest("Center pts position is kept aligned when zooming out via the wheel.");
    ControlDown();
    TimelineTriggerWheel(1);
    ControlUp();
    ASSERT_EQUALS(getTimeline().getZoom().getCurrent(), rational(1,1));
    ASSERT_EQUALS(getTimeline().getScrolling().getCenterPts(), center);
}

void TestScrolling::testShowScrollbarWhenAddingClipAtEnd()
{
    StartTestSuite();
    StartTest("Move clip beyond end of timeline. Scrollbar is shown.");
    ASSERT_EQUALS(getTimeline().GetClientSize(),getTimeline().GetVirtualSize()); // No scrollbars
    pixel length = getTimeline().getSequenceView().getSize().GetWidth();
    Drag(From(Center(VideoClip(0,4))).To(wxPoint(getTimeline().GetRect().GetRight() - 10, VCenter(VideoTrack(0))))); // extend the track to enable scrolling
    ASSERT_DIFFERS(getTimeline().GetClientSize(),getTimeline().GetVirtualSize()); // Scrollbars: Check that the scrolled area != physical area of widget
}

} // namespace