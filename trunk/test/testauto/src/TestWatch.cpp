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

#include "TestWatch.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestWatch::setUp()
{
    if (!HelperTestSuite::get().currentTestIsEnabled()) { return; } // Test was disabled
    mRoot = WindowCreateProject();
    ASSERT(mRoot);
    mInputFiles = getListOfInputPaths();
}

void TestWatch::tearDown()
{
    if (!HelperTestSuite::get().currentTestIsEnabled()) { return; } // Test was disabled
    mRoot.reset();
    mTempDir.reset();
    mInputFiles.clear();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestWatch::testAddAndRemoveFileToWatchedAutoFolder()
{
    StartTestSuite();

    {
        model::FolderPtr folder = setup();

        StartTest("Add supported and valid file on disk");

        wxString aviFileName = mInputFiles.front()->getPath().GetLongPath();
        wxFileName filepath(mTempDirName.GetLongPath(),"valid","avi");
        bool copyok = wxCopyFile( aviFileName, filepath.GetLongPath(), false );
        ASSERT(copyok);
        WaitForChildCount(mRoot, 3);

        ASSERT_WATCHED_PATHS_COUNT(1); // Only the topmost autofolder is being watched
        ASSERT_EQUALS(mRoot->find(util::path::toPath(mTempDirName)).size(),1); // Full path for topmost autofolder
        ASSERT_EQUALS(mRoot->find(util::path::toName(filepath)).size(),1); // File is present

        StartTest("Remove file on disk");
        bool removeok = wxRemoveFile(filepath.GetFullPath());
        ASSERT(removeok);
        WaitForChildCount(mRoot, 2);

        ProjectViewRemove( folder );
        ASSERT_WATCHED_PATHS_COUNT(0); // Nothing is being watched
    }
    {
        StartTest("Add supported but not valid file on disk");
        model::FolderPtr folder = setup();

        wxFileName filepath(mTempDirName.GetLongPath(),"invalid","avi");
        wxFFile aviFile1( filepath.GetLongPath(), "w" );
        aviFile1.Write( "Dummy Contents", wxFile::read_write );
        aviFile1.Close();

        // Add another file to make sure we've waited long enough
        wxString aviFileName = mInputFiles.front()->getPath().GetLongPath();
        wxFileName filepath2(mTempDirName.GetLongPath(),"valid","avi");
        bool copyok = wxCopyFile( aviFileName, filepath2.GetLongPath(), false );
        ASSERT(copyok);
        WaitForChildCount(mRoot, 3); // Only one of the two files is added

        ProjectViewRemove( folder );
        ASSERT_WATCHED_PATHS_COUNT(0); // Nothing is being watched
    }
}

void TestWatch::testRemovedWatchedFolder()
{
    StartTestSuite();
    ASSERT_WATCHED_PATHS_COUNT(0); // Nothing is being watched
    model::FolderPtr folder = setup();

    StartTest("Remove autofolder root dir");
    gui::Dialog::get().setConfirmation();
    mTempDir.reset();
    folder.reset();

    WaitForChildCount(mRoot, 1);

    ASSERT_WATCHED_PATHS_COUNT(0); // Nothing is being watched
    ASSERT_EQUALS(mRoot->find(util::path::toPath(mTempDirName)).size(),0); // Full path for topmost autofolder
}

void TestWatch::testAddAndRemoveFileToWatchedNonAutoFolder()
{
    StartTestSuite();
    model::FolderPtr folder1 = ProjectViewAddFolder( "TestFolder" );
    WaitForChildCount(mRoot, 2);

    StartTest("Add two files to a non-auto folder");
    RandomTempDirPtr tempDir = RandomTempDir::generate();
    wxFileName filepath1_input(getTestFilesPath().GetFullPath(), "01.avi");
    wxFileName filepath2_input(getTestFilesPath().GetFullPath(), "03.avi");
    wxFileName filepath1(tempDir->getFileName().GetLongPath(), "01.avi");
    wxFileName filepath2(tempDir->getFileName().GetLongPath(), "03.avi");
    bool copyok = wxCopyFile( filepath1_input.GetLongPath(), filepath1.GetLongPath(), false );
    ASSERT(copyok);
    copyok = wxCopyFile( filepath2_input.GetLongPath(), filepath2.GetLongPath(), false );
    ASSERT(copyok);

    model::Files files1 = ProjectViewAddFiles( boost::assign::list_of(filepath1)(filepath2), folder1 );
    WaitForChildCount(mRoot, 4);
    ASSERT_WATCHED_PATHS_COUNT(1);

    StartTest("Remove one file (on disk) that is child of a non-auto folder - one other file remains");
    gui::Dialog::get().setConfirmation();
    bool removeok = wxRemoveFile(filepath1.GetFullPath());
    ASSERT(removeok);
    WaitForChildCount(mRoot, 3);
    ASSERT_WATCHED_PATHS_COUNT(1);

    StartTest("Remove last file (on disk) that is child of a non-auto folder - no other file remains");
    gui::Dialog::get().setConfirmation();
    removeok = wxRemoveFile(filepath2.GetFullPath());
    ASSERT(removeok);
    WaitForChildCount(mRoot, 2);
    ASSERT_WATCHED_PATHS_COUNT(0); // Nothing is being watched anymore
}

void TestWatch::testRemoveProjectViewFolderContainingFileOnDisk()
{
    StartTestSuite();
    model::FolderPtr folder1 = ProjectViewAddFolder( "TestFolder2" );
    WaitForChildCount(mRoot, 2);

    StartTest("Add file to a non-auto folder");
    RandomTempDirPtr tempDir = RandomTempDir::generate();
    wxFileName filepath1(tempDir->getFileName().GetLongPath(), "01.avi");
    bool copyok = wxCopyFile( mInputFiles.front()->getPath().GetLongPath(), filepath1.GetLongPath(), false );
    ASSERT(copyok);

    model::Files files1 = ProjectViewAddFiles( boost::assign::list_of(filepath1), folder1 );
    WaitForChildCount(mRoot, 3);
    ASSERT_WATCHED_PATHS_COUNT(1);

    StartTest("Remove folder containing watched file");
    ProjectViewRemove(folder1);
    ASSERT_WATCHED_PATHS_COUNT(0); // Watch for the file must be removed

    StartTest("Add folder again (undo remove)");
    Undo();
    ASSERT_WATCHED_PATHS_COUNT(1); // Watch for the file must be added again

    StartTest("Remove the folder again (redo remove)");
    Redo();
    ASSERT_WATCHED_PATHS_COUNT(0); // Watch for the file must be removed
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

model::FolderPtr TestWatch::setup()
{
    ASSERT_WATCHED_PATHS_COUNT(0); // Nothing is being watched
    mTempDir = RandomTempDir::generate();
    mTempDirName = mTempDir->getFileName();
    model::FolderPtr autofolder = ProjectViewAddAutoFolder( mTempDir->getFileName() ); // Also waits for work
    WaitForChildCount(mRoot, 2);

    ASSERT_EQUALS(mRoot->find(util::path::toPath(mTempDirName)).size(),1); // Full path for topmost autofolder
    ASSERT_WATCHED_PATHS_COUNT(1); // Only the topmost autofolder is being watched
    return autofolder;
}

} // namespace