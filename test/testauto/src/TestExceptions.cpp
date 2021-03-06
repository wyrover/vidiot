// Copyright 2013-2016 Eric Raijmakers.
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

void TestExceptions::testRemoveFileInSequence_Video()
{
    StartTestSuite();
    testRemovedFileInSequence(getFileName("input", "00.avi"));
}

void TestExceptions::testRemovedFileInSequence_Title()
{
    StartTestSuite();
    testRemovedFileInSequence(getFileName("filetypes_title","title.png"));
}

void TestExceptions::testRemovedFileInSequenceBeforeOpening_Video()
{
    StartTestSuite();
    testRemovedFileInSequenceBeforeOpening(getFileName("input", "00.avi"));
}

void TestExceptions::testRemovedFileInSequenceBeforeOpening_Title()
{
    StartTestSuite();
    testRemovedFileInSequenceBeforeOpening(getFileName("filetypes_title","title.png"));
}

void TestExceptions::testRemovedFileUsedForTransitionsBeforeOpening()
{
    StartTestSuite();
    RandomTempDirPtr tempDir = RandomTempDir::generate();
    wxFileNames fileNames;
    {
        model::FolderPtr root = WindowCreateProject();
        ASSERT(root);
        ASSERT_WATCHED_PATHS_COUNT(0);

        StartTest("Create document with transitions");
        model::IPaths InputFiles = getListOfInputPaths();
        for (model::IPathPtr path : getListOfInputPaths())
        {
            wxFileName filepath(tempDir->getFileName().GetLongPath(), path->getPath().GetFullName());
            bool copyok = wxCopyFile(InputFiles.front()->getPath().GetLongPath(), filepath.GetLongPath(), false);
            ASSERT(copyok);

            fileNames.emplace_back(filepath);
        }
        model::FolderPtr folder = ProjectViewAddAutoFolder(tempDir->getFileName());
        WaitForChildCount(root, 2 + InputFiles.size());
        ASSERT_WATCHED_PATHS_COUNT(1);
        model::SequencePtr sequence = ProjectViewCreateSequence(folder);
        WaitForChildCount(root, 3 + InputFiles.size());

        TimelineZoomIn(6);
        MakeInOutTransitionAfterClip t2(1); t2.dontUndo();
        MakeInOutTransitionAfterClip t1(0); t1.dontUndo();
    }
    
    std::pair<RandomTempDirPtr, wxFileName> tempDir_fileName = SaveProjectAndClose();

    wxFileNames::iterator it = fileNames.begin();
    ++it; // Need second file
    wxRemoveFile(it->GetLongPath());
    fileNames.erase(it);

    StartTest("Load document");
    WindowTriggerMenu(wxID_FILE1); // Load document 1 from the file history, this is the file that was saved before. This mechanism avoids the open dialog.
    WaitForIdle;

    Scrub(HCenter(VideoClip(0,1)),RightPixel(VideoClip(0,1)) + 5);
    Scrub(-5 + LeftPixel(VideoClip(0,3)),HCenter(VideoClip(0,3)));

    CloseProjectAndAvoidSaveDialog(); // Avoid files being in use when deleted
}

void TestExceptions::testRemovedFileWithAdjustedSpeed()
{
    StartTestSuite();
    testRemovedFileInSequence(getFileName("input", "00.avi"), []
    {
        TimelineLeftClick(Center(VideoClip(0, 0)));
        ASSERT(DetailsView(VideoClip(0, 0)));
        ASSERT(DetailsClipView()->getSpeedSlider()->IsEnabled());
        SetValue(DetailsClipView()->getSpeedSlider(), 10050);
        ASSERT_CLIP_SPEED(AudioClip(0, 0), rational64(3, 2)); // At this speed the soundtouch calculation is used
    });
}

void TestExceptions::testRemovedFileInProjectViewBeforeOpening()
{
    StartTestSuite();
    RandomTempDirPtr tempDir = RandomTempDir::generate();
    {
        model::FolderPtr root = WindowCreateProject();
        ASSERT(root);
        model::IPaths InputFiles = getListOfInputPaths();

        StartTest("Add file to a non-auto folder");
        wxFileName filepath1(tempDir->getFileName().GetLongPath(), "removedfile.avi");
        bool copyok = wxCopyFile(InputFiles.front()->getPath().GetLongPath(), filepath1.GetLongPath(), false);
        ASSERT(copyok);

        ASSERT_WATCHED_PATHS_COUNT(0);
        model::FolderPtr folder1 = ProjectViewAddFolder("TestFolder");
        WaitForChildCount(root, 2);
        model::Files files1 = ProjectViewAddFiles({ filepath1 }, folder1);
        WaitForChildCount(root, 3);
        ASSERT_WATCHED_PATHS_COUNT(1);
    }

    std::pair<RandomTempDirPtr, wxFileName> tempDir_fileName = SaveProjectAndClose();

    tempDir.reset(); // Deletes the file (still used in the project view) from disk
    StartTest("Load document");
    gui::Dialog::get().setConfirmation(); // A confirmation for the dialog showing that the removed file is deleted from project
    WindowTriggerMenu(wxID_FILE1); // Load document 1 from the file history, this is the file that was saved before. This mechanism avoids the open dialog.
    WaitForIdle;
}

void TestExceptions::testRemovedFolderInProjectViewBeforeOpening()
{
    StartTestSuite();
    RandomTempDirPtr tempDir = RandomTempDir::generate();
    {
        model::FolderPtr root = WindowCreateProject();
        ASSERT(root);

        model::FolderPtr autofolder = ProjectViewAddAutoFolder(tempDir->getFileName()); // Also waits for work
        WaitForChildCount(root, 2);
    }

    std::pair<RandomTempDirPtr, wxFileName> tempDir_fileName = SaveProjectAndClose();

    tempDir.reset(); // Deletes the folder (still used in the project view) from disk
    StartTest("Load document");
    gui::Dialog::get().setConfirmation(); // A confirmation for the dialog showing that the removed file is deleted from project
    WindowTriggerMenu(wxID_FILE1); // Load document 1 from the file history, this is the file that was saved before. This mechanism avoids the open dialog.
    WaitForIdle;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

std::pair< model::SequencePtr, RandomTempDirPtr> TestExceptions::createProjectWithOneFile(const wxFileName& file)
{
    model::FolderPtr root = WindowCreateProject();
    ASSERT(root);
    model::IPaths InputFiles = getListOfInputPaths();

    StartTest("Add file to a non-auto folder");
    RandomTempDirPtr tempDir = RandomTempDir::generate();
    wxFileName filepath1(tempDir->getFileName().GetLongPath(), "removedfile." + file.GetExt());
    bool copyok = wxCopyFile( file.GetLongPath(), filepath1.GetLongPath(), false );
    ASSERT(copyok);

    ASSERT_WATCHED_PATHS_COUNT(0);
    model::FolderPtr folder1 = ProjectViewAddFolder( "TestFolder" );
    WaitForChildCount(root, 2);
    model::Files files1 = ProjectViewAddFiles({ filepath1 }, folder1);
    WaitForChildCount(root, 3);
    ASSERT_WATCHED_PATHS_COUNT(1);
    model::SequencePtr sequence = ProjectViewCreateSequence(folder1);
    WaitForChildCount(root, 4);
    ProjectViewRemove(folder1);
    ASSERT_WATCHED_PATHS_COUNT(0);
    return std::make_pair(sequence, tempDir);
}

void TestExceptions::testRemovedFileInSequence(const wxFileName& file, std::function<void()> adjustment)
{
    std::pair< model::SequencePtr, RandomTempDirPtr> projectfolder_and_dirtoberemoved = createProjectWithOneFile(file);
    if (adjustment)
    {
        adjustment();
    }
    WindowTriggerMenu(ID_CLOSESEQUENCE);

    projectfolder_and_dirtoberemoved.second.reset(); // Deletes the file (still used in the sequence) from disk

    // Open the sequence again (file missing from disk)
    util::thread::RunInMainAndWait([projectfolder_and_dirtoberemoved]
    {
        GetProjectView().select({ projectfolder_and_dirtoberemoved.first });
        GetProjectView().onOpen();
    });

    Play(10, 500);
}

void TestExceptions::testRemovedFileInSequenceBeforeOpening(const wxFileName& file)
{
    std::pair< model::SequencePtr, RandomTempDirPtr> projectfolder_and_dirtoberemoved = createProjectWithOneFile(file);
    std::pair<RandomTempDirPtr, wxFileName> tempDir_fileName = SaveProjectAndClose();

    projectfolder_and_dirtoberemoved.second.reset(); // Deletes the file (still used in the sequence) from disk

    StartTest("Load document");
    WindowTriggerMenu(wxID_FILE1); // Load document 1 from the file history, this is the file that was saved before. This mechanism avoids the open dialog.
    WaitForIdle;

    Play(10, 500);
}

wxFileName TestExceptions::getFileName(wxString folder, wxString file) const
{
    wxFileName path = getTestPath();
    path.AppendDir(folder);
    ASSERT(path.IsDir());
    ASSERT(path.DirExists());
    path.SetFullName(file);
    ASSERT(path.FileExists());
    return path;
}



} // namespace
