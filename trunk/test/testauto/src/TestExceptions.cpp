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

#include "TestExceptions.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestExceptions::testRemovedFileInSequence()
{
    StartTestSuite();
    model::FolderPtr root = WindowCreateProject();
    ASSERT(root);
    model::IPaths InputFiles = getListOfInputFiles();

    StartTest("Add file to a non-auto folder");
    RandomTempDirPtr tempDir = RandomTempDir::generate();
    wxFileName filepath1(tempDir->getFileName().GetLongPath(), "removedfile.avi");
    bool copyok = wxCopyFile( InputFiles.front()->getPath().GetLongPath(), filepath1.GetLongPath(), false );
    ASSERT(copyok);

    ASSERT_WATCHED_PATHS_COUNT(0);
    model::FolderPtr folder1 = ProjectViewAddFolder( "TestFolder" );
    WaitForChildCount(root, 2);
    model::Files files1 = ProjectViewAddFiles( boost::assign::list_of(filepath1), folder1 );
    WaitForChildCount(root, 3);
    ASSERT_WATCHED_PATHS_COUNT(1);
    model::SequencePtr sequence = ProjectViewCreateSequence(folder1);
    WaitForChildCount(root, 4);
    ProjectViewRemove(folder1);
    ASSERT_WATCHED_PATHS_COUNT(0);
    WindowTriggerMenu(ID_CLOSESEQUENCE);

    tempDir.reset(); // Deletes the file (still used in the sequence) from disk

    // Open the sequence again (file missing from disk)
    RunInMainAndWait([folder1]
    {
        GetProjectView().select(boost::assign::list_of(folder1));
    });
    RunInMainAndWait([]
    {
        GetProjectView().onOpen();
    });

    Play(10, 500);
}

void TestExceptions::testRemovedFileInSequenceBeforeOpening()
{
    StartTestSuite();
    model::FolderPtr root = WindowCreateProject();
    ASSERT(root);
    model::IPaths InputFiles = getListOfInputFiles();

    StartTest("Add file to a non-auto folder");
    RandomTempDirPtr tempDir = RandomTempDir::generate();
    wxFileName filepath1(tempDir->getFileName().GetLongPath(), "removedfile.avi");
    bool copyok = wxCopyFile( InputFiles.front()->getPath().GetLongPath(), filepath1.GetLongPath(), false );
    ASSERT(copyok);

    ASSERT_WATCHED_PATHS_COUNT(0);
    model::FolderPtr folder1 = ProjectViewAddFolder( "TestFolder" );
    WaitForChildCount(root, 2);
    model::Files files1 = ProjectViewAddFiles( boost::assign::list_of(filepath1), folder1 );
    WaitForChildCount(root, 3);
    ASSERT_WATCHED_PATHS_COUNT(1);
    model::SequencePtr sequence = ProjectViewCreateSequence(folder1);
    WaitForChildCount(root, 4);
    ProjectViewRemove(folder1);
    ASSERT_WATCHED_PATHS_COUNT(0);

    std::pair<RandomTempDirPtr, wxFileName> tempDir_fileName = SaveProjectAndClose();

    tempDir.reset(); // Deletes the file (still used in the sequence) from disk

    StartTest("Load document");
    WindowTriggerMenu(wxID_FILE1); // Load document 1 from the file history, this is the file that was saved before. This mechanism avoids the open dialog.
    WaitForIdle();

    Play(10, 500);
}

void TestExceptions::testRemovedFileUsedForTransitionsBeforeOpening()
{
    StartTestSuite();

    model::FolderPtr root = WindowCreateProject();
    ASSERT(root);
    ASSERT_WATCHED_PATHS_COUNT(0);

    StartTest("Create document with transitions");
    model::IPaths InputFiles = getListOfInputFiles();
    RandomTempDirPtr tempDir = RandomTempDir::generate();
    std::list<wxFileName> fileNames;
    for ( model::IPathPtr path : getListOfInputFiles() )
    {
        wxFileName filepath(tempDir->getFileName().GetLongPath(), path->getPath().GetFullName());
        bool copyok = wxCopyFile( InputFiles.front()->getPath().GetLongPath(), filepath.GetLongPath(), false );
        ASSERT(copyok);

        fileNames.push_back(filepath);
    }
    model::FolderPtr folder = ProjectViewAddAutoFolder( tempDir->getFileName() );
    WaitForChildCount(root, 2 + InputFiles.size());
    ASSERT_WATCHED_PATHS_COUNT(1);
    model::SequencePtr sequence = ProjectViewCreateSequence(folder);
    WaitForChildCount(root, 3 + InputFiles.size());

    Zoom level(6);
    MakeInOutTransitionAfterClip t2(1); t2.dontUndo();
    MakeInOutTransitionAfterClip t1(0); t1.dontUndo();

    std::pair<RandomTempDirPtr, wxFileName> tempDir_fileName = SaveProjectAndClose();

    std::list<wxFileName>::iterator it = fileNames.begin();
    ++it; // Need second file
    wxRemoveFile(it->GetLongPath());
    fileNames.erase(it);

    StartTest("Load document");
    WindowTriggerMenu(wxID_FILE1); // Load document 1 from the file history, this is the file that was saved before. This mechanism avoids the open dialog.
    WaitForIdle();

    Scrub(HCenter(VideoClip(0,1)),RightPixel(VideoClip(0,1)) + 5);
    Scrub(-5 + LeftPixel(VideoClip(0,3)),HCenter(VideoClip(0,3)));

    CloseDocumentAndAvoidSaveDialog(); // Avoid files being in use when deleted
}

void TestExceptions::testRemovedFileInProjectViewBeforeOpening()
{
    StartTestSuite();
    model::FolderPtr root = WindowCreateProject();
    ASSERT(root);
    model::IPaths InputFiles = getListOfInputFiles();

    StartTest("Add file to a non-auto folder");
    RandomTempDirPtr tempDir = RandomTempDir::generate();
    wxFileName filepath1(tempDir->getFileName().GetLongPath(), "removedfile.avi");
    bool copyok = wxCopyFile( InputFiles.front()->getPath().GetLongPath(), filepath1.GetLongPath(), false );
    ASSERT(copyok);

    ASSERT_WATCHED_PATHS_COUNT(0);
    model::FolderPtr folder1 = ProjectViewAddFolder( "TestFolder" );
    WaitForChildCount(root, 2);
    model::Files files1 = ProjectViewAddFiles( boost::assign::list_of(filepath1), folder1 );
    WaitForChildCount(root, 3);
    ASSERT_WATCHED_PATHS_COUNT(1);

    std::pair<RandomTempDirPtr, wxFileName> tempDir_fileName = SaveProjectAndClose();

    tempDir.reset(); // Deletes the file (still used in the project view) from disk

    StartTest("Load document");
    gui::Dialog::get().setConfirmation(); // A confirmation for the dialog showing that the removed file is deleted from project
    WindowTriggerMenu(wxID_FILE1); // Load document 1 from the file history, this is the file that was saved before. This mechanism avoids the open dialog.
    WaitForIdle();
}

void TestExceptions::testRemovedFolderInProjectViewBeforeOpening()
{
    StartTestSuite();
    model::FolderPtr root = WindowCreateProject();
    ASSERT(root);

    RandomTempDirPtr tempDir = RandomTempDir::generate();
    RandomTempDirPtr subDir = tempDir->generateSubDir();
    RandomTempDirPtr subSubDir = subDir->generateSubDir();
    model::FolderPtr autofolder = ProjectViewAddAutoFolder( tempDir->getFileName() ); // Also waits for work
    WaitForChildCount(root, 4);

    std::pair<RandomTempDirPtr, wxFileName> tempDir_fileName = SaveProjectAndClose();

    tempDir.reset(); // Deletes the folder (still used in the project view) from disk

    StartTest("Load document");
    gui::Dialog::get().setConfirmation(); // A confirmation for the dialog showing that the removed file is deleted from project
    WindowTriggerMenu(wxID_FILE1); // Load document 1 from the file history, this is the file that was saved before. This mechanism avoids the open dialog.
    WaitForIdle();
}

} // namespace