// Copyright 2014-2016 Eric Raijmakers.
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

#include "TestUiBugs.h"
//todo make testui work on linux
namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestUiBugs::setUp()
{
    mProjectFixture.init();
}

void TestUiBugs::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestUiBugs::testDragAndDropVideoFileLargerThanTimeline()
{
    StartTestSuite();
    TimelineZoomIn(2);

    wxFileName file = getTestPath();
    file.AppendDir("filetypes_special");
    ASSERT(file.IsDir());
    ASSERT(file.DirExists());
    file.SetFullName("long_black_10min.avi"); // long file
    ASSERT(file.FileExists());

    model::Files files = ProjectViewAddFiles({ file });

    DragFromProjectViewToTimeline(files.front(), getTimeline().GetScreenPosition() + wxPoint(5, VCenter(VideoTrack(0))));
    ASSERT_EQUALS(NumberOfVideoClipsInTrack(0), 1); // Dropped clip obscured entire track
    ASSERT_EQUALS(NumberOfAudioClipsInTrack(0), 7);
    ASSERT_MORE_THAN_EQUALS(VideoClip(0)->getLength(), model::Convert::timeToPts(10 * 60 * 1000));
}

void TestUiBugs::testDragAndDropAudioFileLargerThanTimeline()
{
    StartTestSuite();
    TimelineZoomIn(2);

    wxFileName file = getTestPath();
    file.AppendDir("filetypes_special");
    ASSERT(file.IsDir());
    ASSERT(file.DirExists());
    file.SetFullName("Dawn - Another Day.mp3"); // long file
    ASSERT(file.FileExists());

    model::Files files = ProjectViewAddFiles({ file });

    DragFromProjectViewToTimeline(files.front(), getTimeline().GetScreenPosition() + wxPoint(5, VCenter(AudioTrack(0))));
    ASSERT_EQUALS(NumberOfVideoClipsInTrack(0), 7);
    ASSERT_EQUALS(NumberOfAudioClipsInTrack(0), 1); // Dropped clip obscured entire track
    ASSERT_MORE_THAN_EQUALS(AudioClip(0)->getLength(), model::Convert::timeToPts(3 * 60 * 1000 + 27 * 1000));
}

void TestUiBugs::testCrashWhenRightClickingOutsideAllTracks()
{
    StartTestSuite();
    TimelineRightClick(wxPoint(200, gui::timeline::TimescaleView::TimeScaleHeight + 2));
    TimelineKeyPress(WXK_ESCAPE);
}

} // namespace
