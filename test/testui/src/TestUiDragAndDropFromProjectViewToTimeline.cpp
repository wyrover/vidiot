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

#include "TestUiDragAndDropFromProjectViewToTimeline.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestUiDragAndDropFromProjectViewToTimeline::setUp()
{
    mProjectFixture.init();
}

void TestUiDragAndDropFromProjectViewToTimeline::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestUiDragAndDropFromProjectViewToTimeline::testDragAndDropIntoEmptySequence()
{
    StartTestSuite();
    OpenFolderWithInputFiles();
    TriggerMenu(ID_CLOSESEQUENCE);
    wxString sSequence1( "Sequence1" );
    model::SequencePtr sequence1 = addSequence( sSequence1, mProjectFixture.mRoot );
    model::NodePtrs fileNodes = mProjectFixture.mRoot->find("02.avi");
    DragFromProjectViewToTimeline(fileNodes.front(),  getTimeline().GetScreenPosition() - getTimeline().getScrolling().getOffset() + wxPoint(5, VCenter(VideoTrack(0))) );
    ASSERT_EQUALS(NumberOfVideoClipsInTrack(0),1);
    ASSERT_VIDEOTRACK0(VideoClip);
}

void TestUiDragAndDropFromProjectViewToTimeline::testDragAndDropAtEndOfSequence()
{
    StartTestSuite();
    OpenFolderWithInputFiles();
    StartTest("Execute drag and drop: Drop clip onto last clip.");
    model::NodePtrs fileNodes = mProjectFixture.mRoot->find("02.avi");
    ASSERT_EQUALS(fileNodes.size(),1);
    pts trackLength = VideoTrack(0)->getLength();
    pts cliplength = VideoClip(0,2)->getLength();
    DragFromProjectViewToTimeline(fileNodes.front(),  getTimeline().GetScreenPosition() - getTimeline().getScrolling().getOffset() + wxPoint(RightPixel(VideoTrack(0)), VCenter(VideoTrack(0))) );
    ASSERT_EQUALS(NumberOfVideoClipsInTrack(0),8);
    ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
    ASSERT_MORE_THAN(VideoTrack(0)->getLength(),trackLength);
    ASSERT_EQUALS(VideoClip(0,-1)->getLength(), VideoClip(0,2)->getLength());
    StartTest("Undo drop.");
    Undo();
    ASSERT_EQUALS(NumberOfVideoClipsInTrack(0),7);
    StartTest("Execute drag and drop: Drop clip after last clip.");
    DragFromProjectViewToTimeline(fileNodes.front(),  getTimeline().GetScreenPosition() - getTimeline().getScrolling().getOffset() + wxPoint(RightPixel(VideoTrack(0)), VCenter(VideoTrack(0))) + wxPoint(150,0));
    ASSERT_EQUALS(NumberOfVideoClipsInTrack(0),9);
    ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(EmptyClip)(VideoClip);
    ASSERT_MORE_THAN(VideoTrack(0)->getLength(),trackLength);
    ASSERT_EQUALS(VideoClip(0,-1)->getLength(), VideoClip(0,2)->getLength());
    StartTest("Undo drop.");
    Undo();
    ASSERT_EQUALS(NumberOfVideoClipsInTrack(0),7);
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void TestUiDragAndDropFromProjectViewToTimeline::OpenFolderWithInputFiles()
{
    StartTest("Open the folder in the project view."); // Required for starting the drag: the node must be visible
    model::NodePtrs nodes = mProjectFixture.mRoot->getChildren();
    model::NodePtr folder;
    for ( model::NodePtr node : mProjectFixture.mRoot->getChildren() )
    {
        if (node->isA<model::Folder>())
        {
            folder = node;
            break;
        }
    }
    ASSERT_NONZERO(folder);
    MouseMoveOnScreen(CenterInProjectView(folder));
    MouseLeftDown();
    MouseLeftUp();
    KeyboardKeyPress(WXK_RIGHT);
}

// todo test case for each possible key stroke in the timeline popup menu
// example 't':
//void TestBugs::testDeleteClipInbetweenTransitionsCausesTimelineMessUp()
//{
//    StartTestSuite();
//    Zoom level(6);
//    MakeInOutTransitionAfterClip t1(1);
//    MakeInOutTransitionAfterClip t2(0);
//    OpenPopupMenuAt(Center(VideoClip(0,2)));
//    TimelineKeyPress('t');
//    ASSERT_EQUALS(VideoClip(0,1)->getLeftPts(), AudioClip(0,1)->getLeftPts());
//    ASSERT_EQUALS(VideoTrack(0)->getLength(),AudioTrack(0)->getLength());
//    ASSERT_EQUALS(VideoClip(0,4)->getRightPts(), AudioClip(0,4)->getRightPts());
//    Undo();
//}

} // namespace
