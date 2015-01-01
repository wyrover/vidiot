// Copyright 2013-2015 Eric Raijmakers.
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

#include "TestUiTimelinePopupMenuRightClickSelection.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestUiTimelinePopupMenuRightClickSelection::setUp()
{
    mProjectFixture.init();
}

void TestUiTimelinePopupMenuRightClickSelection::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestUiTimelinePopupMenuRightClickSelection::testRightClickSelection()
{
    StartTestSuite();
    TimelineZoomIn(4);

    {
        DeselectAllClips();
        MakeInOutTransitionAfterClip preparation(1);
        StartTest("InOutTransition: Right Clicking on TransitionLeftClipInterior selects the clip left of the transition.");
        TimelineOpenPopupMenuAt(TransitionLeftClipInterior(VideoClip(0,2)));
        ASSERT(VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        TimelineClosePopupMenu();
        StartTest("InOutTransition: Right Clicking on TransitionRightClipInterior selects the clip right of the transition.");
        TimelineOpenPopupMenuAt(TransitionRightClipInterior(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(VideoClip(0,3)->getSelected());
        TimelineClosePopupMenu();
        StartTest("InOutTransition: Right Clicking on TransitionLeftClipEnd selects the clip left of the transition.");
        TimelineOpenPopupMenuAt(TransitionLeftClipEnd(VideoClip(0,2)));
        ASSERT(VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        TimelineClosePopupMenu();
        StartTest("InOutTransition: Right Clicking on TransitionRightClipBegin selects the clip right of the transition.");
        TimelineOpenPopupMenuAt(TransitionRightClipBegin(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(VideoClip(0,3)->getSelected());
        TimelineClosePopupMenu();
    }
    {
        DeselectAllClips();
        MakeOutTransitionAfterClip preparation(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        StartTest("OutTransition: Right Clicking on TransitionLeftClipInterior selects the clip left of the transition.");
        TimelineOpenPopupMenuAt(TransitionLeftClipInterior(VideoClip(0,2)));
        ASSERT(VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        TimelineClosePopupMenu();
        StartTest("OutTransition: Right Clicking on TransitionLeftClipEnd selects the clip left of the transition.");
        TimelineOpenPopupMenuAt(TransitionLeftClipEnd(VideoClip(0,2)));
        ASSERT(VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(!VideoClip(0,3)->getSelected());
        TimelineClosePopupMenu();
    }
    {
        DeselectAllClips();
        MakeInTransitionAfterClip preparation(1);
        ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(Transition)(VideoClip)(VideoClip);
        StartTest("InTransition: Right Clicking on TransitionRightClipInterior selects the clip right of the transition.");
        TimelineOpenPopupMenuAt(TransitionRightClipInterior(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(VideoClip(0,3)->getSelected());
        TimelineClosePopupMenu();
        StartTest("InTransition: Right Clicking on TransitionRightClipBegin selects the clip right of the transition.");
        TimelineOpenPopupMenuAt(TransitionRightClipBegin(VideoClip(0,2)));
        ASSERT(!VideoClip(0,1)->getSelected());
        ASSERT(!VideoClip(0,2)->getSelected());
        ASSERT(VideoClip(0,3)->getSelected());
        TimelineClosePopupMenu();
    }
}

} // namespace