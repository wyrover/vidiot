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

#include "Cursor.h"
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

void TestScrolling::testUpdateScrollingIfCursorIsMovedOutsideVisibleRegion()
{
    auto ASSERT_SCROLLING_POSITION = [](pixel positionInVisibleArea, pts expected)
    {
        wxSize s = getTimeline().GetClientSize();
        wxPoint p = getTimeline().getScrolling().getOffset();
        if (positionInVisibleArea >= 0)
        {
            ASSERT_EQUALS(getTimeline().getZoom().pixelsToPts(p.x + positionInVisibleArea), expected);
        }
        else
        {
            ASSERT_EQUALS(getTimeline().getZoom().pixelsToPts(p.x + s.x + positionInVisibleArea), expected);
        }
    };
    StartTestSuite();
    Zoom level(6);
    StartTest("Move cursor to end of timeline.");
    Type(WXK_END);
    ASSERT_SCROLLING_POSITION(- gui::timeline::Cursor::EDGE_OFFSET, getSequence()->getLength());
    ASSERT_EQUALS(getTimeline().getCursor().getLogicalPosition(), getSequence()->getLength());
    StartTest("Cursor not moved when not holding CTRL.");
    Type(WXK_LEFT);
    StartTest("Move cursor to previous cut.");
    ControlDown();
    Type(WXK_LEFT);
    ControlUp();
    ASSERT_EQUALS(getTimeline().getCursor().getLogicalPosition(), VideoClip(0,6)->getLeftPts());
    ASSERT_SCROLLING_POSITION(- gui::timeline::Cursor::EDGE_OFFSET, getSequence()->getLength()); // Not changed
    ControlDown();
    Type(WXK_LEFT);
    ControlUp();
    ASSERT_EQUALS(getTimeline().getCursor().getLogicalPosition(), VideoClip(0,5)->getLeftPts());
    ASSERT_SCROLLING_POSITION(- gui::timeline::Cursor::EDGE_OFFSET, getSequence()->getLength()); // Not changed
    StartTest("Move cursor to previous cut and beyond the timeline visible area begin.");
    ControlDown();
    Type(WXK_LEFT);
    ControlUp();
    ASSERT_EQUALS(getTimeline().getCursor().getLogicalPosition(), VideoClip(0,4)->getLeftPts());
    ASSERT_SCROLLING_POSITION(gui::timeline::Cursor::EDGE_OFFSET, VideoClip(0,4)->getLeftPts());
    StartTest("Cursor not moved when not holding CTRL.");
    Type(WXK_RIGHT);
    StartTest("Move cursor to next cut.");
    ControlDown();
    Type(WXK_RIGHT);
    ControlUp();
    ASSERT_EQUALS(getTimeline().getCursor().getLogicalPosition(), VideoClip(0,5)->getLeftPts());
    ASSERT_SCROLLING_POSITION(gui::timeline::Cursor::EDGE_OFFSET, VideoClip(0,4)->getLeftPts()); // Not changed
    StartTest("Move cursor to next cut and beyond the timeline visible area end.");
    ControlDown();
    Type(WXK_RIGHT);
    ControlUp();
    ASSERT_EQUALS(getTimeline().getCursor().getLogicalPosition(), VideoClip(0,6)->getLeftPts());
    ASSERT_SCROLLING_POSITION(- gui::timeline::Cursor::EDGE_OFFSET, VideoClip(0,6)->getLeftPts());
    StartTest("Move cursor to begin of timeline.");
    ControlDown();
    Type(WXK_HOME);
    ControlUp();
    ASSERT_EQUALS(getTimeline().getCursor().getLogicalPosition(), 0);
    ASSERT_SCROLLING_POSITION(0, 0);
}

} // namespace