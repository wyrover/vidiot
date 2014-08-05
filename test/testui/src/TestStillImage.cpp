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

#include "TestStillImage.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestStillImage::setUp()
{
    mProjectFixture.init();
}

void TestStillImage::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestStillImage::testUndo()
{
    StartTestSuite();

    wxString sFolder1( "Folder1" );
    model::FolderPtr folder1 = ProjectViewAddFolder( sFolder1 );
    wxFileName TestFilesPath = getTestPath();
    TestFilesPath.AppendDir("filetypes_image");
    ASSERT(TestFilesPath.IsDir());
    ASSERT(TestFilesPath.DirExists());
    TestFilesPath.SetFullName("Laney -6th best amp.jpg");
    model::Files files1 = ProjectViewAddFiles( boost::assign::list_of(TestFilesPath.GetFullPath()), folder1 );
    model::FilePtr imageFile = files1.front();

    DragFromProjectViewToTimeline( imageFile,  getTimeline().GetScreenPosition() - getTimeline().getScrolling().getOffset()  + wxPoint(HCenter(VideoClip(0,4)), VCenter(VideoTrack(0))) );

    Scrub(HCenter(VideoClip(0,5)),HCenter(VideoClip(0,5)) + 10);

    TimelineLeftClick(Center(VideoClip(0,5)));
    TimelineKeyPress(WXK_DELETE);

    Undo();

    Scrub(HCenter(VideoClip(0,5)),HCenter(VideoClip(0,5)) + 10);
}

} // namespace
