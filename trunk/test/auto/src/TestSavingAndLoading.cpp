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

#include "TestSavingAndLoading.h"

#include "HelperApplication.h"
#include "HelperFileSystem.h"
#include "HelperProject.h"
#include "HelperTimeline.h"
#include "HelperTimelineTrim.h"
#include "HelperWindow.h"
#include "Project.h"
#include "Window.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestSavingAndLoading::setUp()
{
    mProjectFixture.init();
}

void TestSavingAndLoading::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestSavingAndLoading::testSaveAndLoad()
{
    StartTestSuite();
    StartTest("SetUp");

    wxString sFolder1( "Folder1" );
    model::FolderPtr folder1 = addFolder( sFolder1 );
    wxFileName TestFilesPath = wxFileName(SOURCE_ROOT,"");
    TestFilesPath.AppendDir("test");
    TestFilesPath.AppendDir("filetypes_image");
    ASSERT(TestFilesPath.IsDir());
    ASSERT(TestFilesPath.DirExists());
    TestFilesPath.SetFullName("Laney -6th best amp.jpg");
    model::Files files1 = addFiles( boost::assign::list_of(TestFilesPath.GetFullPath()), folder1 );
    model::FilePtr imageFile = files1.front();

    DragFromProjectViewToTimeline( imageFile,  getTimeline().GetScreenPosition() - getTimeline().getScrolling().getOffset()  + wxPoint(HCenter(VideoClip(0,4)), VCenter(VideoTrack(0))) );

    Click(Center(VideoClip(0,6)));
    Type(WXK_DELETE);

    std::pair<RandomTempDirPtr, wxFileName> tempDir_fileName = SaveProjectAndClose();

    StartTest("Load document");
    triggerMenu(wxID_FILE1); // Load document 1 from the file history, this is the file that was saved before. This mechanism avoids the open dialog.
    waitForIdle();

    StartTest("Trim clip"); // Known bug at some point: a crash due to improper initialization of File class members upon loading (mNumberOfFrames not initialized)
    TrimLeft(VideoClip(0,1),20);
    Undo();

    StartTest("Enlarge sequence"); // Known bug at some point: enlarging the sequence did not cause an update of the timeline virtual size due to missing event binding
    Zoom level(2); // Zoom in twice
    wxSize paneSize = getTimeline().GetVirtualSize();
    wxSize size = getTimeline().getSequenceView().getSize();
    Drag(From(Center(AudioClip(0,7))).To(wxPoint(size.x - 2, VCenter(AudioTrack(0)))));
    wxSize newSize = getTimeline().getSequenceView().getSize();
    ASSERT_DIFFERS(getTimeline().getSequenceView().getSize(), size);
    ASSERT_DIFFERS(getTimeline().GetVirtualSize(), paneSize);

    model::Project::get().Modify(false); // Avoid 'save?' dialog
    StartTest("TearDown");
    triggerMenu(wxID_CLOSE);
}

} // namespace