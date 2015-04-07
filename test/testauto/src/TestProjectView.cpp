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

#include "TestProjectView.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestProjectView::setUp()
{
    mProjectFixture.init();
}

void TestProjectView::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestProjectView::testAdditionAndRemoval()
{
    StartTestSuite();

    int nDefaultItems = ProjectViewCount();

    wxString sFolder1( "Folder1" );
    wxString sSequence1( "Sequence1" );
    wxString sFile( "05.avi" );
    wxFileName filepath(getTestFilesPath().GetFullPath(), sFile);

    model::FolderPtr folder1 = ProjectViewAddFolder( sFolder1 );
    model::SequencePtr sequence1 = ProjectViewAddSequence( sSequence1, folder1 );
    model::Files files1 = ProjectViewAddFiles({ filepath }, folder1);

    ASSERT_EQUALS(folder1->getParent(),mProjectFixture.mRoot);
    ASSERT_EQUALS(sequence1->getParent(),folder1);
    ASSERT_EQUALS(ProjectViewCount(), nDefaultItems + 3); // Added: Folder + Sequence + File
    ASSERT_EQUALS(files1.size(),1);
    ASSERT_EQUALS(files1.front()->getParent(),folder1);
    ASSERT_EQUALS(mProjectFixture.mRoot->find(sFile).size(),1);    // One file with a relative file name
    ASSERT_EQUALS(mProjectFixture.mRoot->find(filepath.GetLongPath()).size(),1); // And one file with an absolute file name

    ProjectViewRemove( files1.front() );
    ASSERT_EQUALS(ProjectViewCount(), nDefaultItems + 2); // Added: Folder + Sequence
    ProjectViewRemove( folder1 ); // Also removes sequence1 which is contained in folder1
    ASSERT_EQUALS(ProjectViewCount(), nDefaultItems); // Added: None
}

void TestProjectView::testClipboardCut()
{
    StartTestSuite();

    {
        ClearClipboard();
        StartTest("Cut from main menu");
        ASSERT_EQUALS(ProjectViewCount(), 3);
        ProjectViewSetFocus();
        ProjectViewSelect(model::Project::get().getRoot()->getChildren());
        WindowTriggerMenu(wxID_CUT);
        ASSERT_EQUALS(ProjectViewCount(), 1);
        ASSERT_CLIPBOARD_CONTAINS_NODES;
    }
}

void TestProjectView::testClipboardCopy()
{
    StartTestSuite();
    {
        ClearClipboard();
        StartTest("Copy from main menu");
        ASSERT_EQUALS(ProjectViewCount(), 3);
        ProjectViewSetFocus();
        ProjectViewSelect(model::Project::get().getRoot()->getChildren());
        WindowTriggerMenu(wxID_COPY);
        ASSERT_EQUALS(ProjectViewCount(), 3);
        ASSERT_CLIPBOARD_CONTAINS_NODES;
    }
}

void TestProjectView::testClipboardPaste_ClipboardEmpty()
{
    StartTestSuite();
    ClearClipboard();
    {
        StartTest("Paste from main menu (nothing selected, nothing in clipboard)");
        ProjectViewSelect(model::NodePtrs());
        WindowTriggerMenu(wxID_PASTE);
        ASSERT_EQUALS(ProjectViewCount(), 3);
    }
    {
        StartTest("Paste from main menu (root selected, nothing in clipboard)");
        ProjectViewSelect({ getRoot() });
        WindowTriggerMenu(wxID_PASTE);
        ASSERT_EQUALS(ProjectViewCount(), 3);
    }
    {
        StartTest("Paste from main menu (folder selected, nothing in clipboard)");
        model::FolderPtr folder = ProjectViewAddFolder("FOLDER");
        ASSERT_EQUALS(ProjectViewCount(), 4);
        ProjectViewSelect({ folder });
        WindowTriggerMenu(wxID_PASTE);
        ASSERT_EQUALS(ProjectViewCount(), 4);
    }
}

void TestProjectView::testClipboardPaste_ClipboardNodes()
{
    StartTestSuite();

    StartTest("Preparation: put nodes in clipboard");
    RandomTempDirPtr tempDir1 = RandomTempDir::generate();
    wxFileName filepath1(tempDir1->getFileName().GetLongPath(), "01.avi");
    bool copyok = wxCopyFile(getListOfInputPaths().at(0)->getPath().GetLongPath(), filepath1.GetLongPath(), false);
    ASSERT(copyok);
    model::NodePtr folder = ProjectViewAddAutoFolder(tempDir1->getFileName());

    RandomTempDirPtr tempDir2 = RandomTempDir::generate();
    wxFileName filepath2(tempDir2->getFileName().GetLongPath(), "02.avi");
    copyok = wxCopyFile(getListOfInputPaths().at(1)->getPath().GetLongPath(), filepath2.GetLongPath(), false);
    ASSERT(copyok);

    model::NodePtr file = ProjectViewAddFiles({ filepath2 }).front();

    ASSERT_EQUALS(ProjectViewCount(), 5);
    ProjectViewSetFocus();
    ProjectViewSelect({ folder, file});
    WindowTriggerMenu(wxID_CUT);
    ASSERT_EQUALS(ProjectViewCount(), 3);
    ASSERT_CLIPBOARD_CONTAINS_NODES;

    {
        StartTest("Paste from main menu (nothing selected, nodes in clipboard)");
        ProjectViewSelect(model::NodePtrs());
        WindowTriggerMenu(wxID_PASTE);
        ASSERT_EQUALS(ProjectViewCount(), 5);
        ASSERT_EQUALS(getRoot()->getChildren().size(), 4);
        Undo();
    }
    {
        StartTest("Paste from main menu (root selected, nodes in clipboard)");
        ProjectViewSelect({ getRoot() });
        WindowTriggerMenu(wxID_PASTE);
        ASSERT_EQUALS(ProjectViewCount(), 5);
        ASSERT_EQUALS(getRoot()->getChildren().size(), 4);
        Undo();
    }
    {
        StartTest("Paste from main menu (folder selected, nodes in clipboard)");
        model::FolderPtr folder = ProjectViewAddFolder("FOLDER");
        ASSERT_EQUALS(ProjectViewCount(), 4);
        ProjectViewSelect({ folder });
        WindowTriggerMenu(wxID_PASTE);
        ASSERT_EQUALS(ProjectViewCount(), 6);
        ASSERT_EQUALS(folder->getChildren().size(), 2);
        Undo(2);
    }

    // Now remove the files on disk and try again
    tempDir1.reset();
    tempDir2.reset();
    {
        StartTest("Paste from main menu (nothing selected, nodes in clipboard, files removed)");
        ProjectViewSelect(model::NodePtrs());
        WindowTriggerMenu(wxID_PASTE);
        ASSERT_EQUALS(ProjectViewCount(), 3);
        ASSERT_EQUALS(getRoot()->getChildren().size(), 2);
    }
    {
        StartTest("Paste from main menu (root selected, nodes in clipboard, files removed)");
        ProjectViewSelect({ getRoot() });
        WindowTriggerMenu(wxID_PASTE);
        ASSERT_EQUALS(ProjectViewCount(), 3);
        ASSERT_EQUALS(getRoot()->getChildren().size(), 2);
    }
    {
        StartTest("Paste from main menu (folder selected, nodes in clipboard, files removed)");
        model::FolderPtr folder = ProjectViewAddFolder("FOLDER");
        ASSERT_EQUALS(ProjectViewCount(), 4);
        ProjectViewSelect({ folder });
        WindowTriggerMenu(wxID_PASTE);
        ASSERT_EQUALS(ProjectViewCount(), 4);
        ASSERT_EQUALS(folder->getChildren().size(), 0);
    }
}

void TestProjectView::testClipboardPaste_ClipboardFiles()
{
	StartTestSuite();
	StartTest("Preparation: Files and one folder in clipboard.");
    wxFileNames files(getListOfInputPathsAsFileNames());
	FillClipboardWithFiles(files);
	ProjectViewSetFocus();
	{
		StartTest("Paste from main menu (nothing selected, files in clipboard)");
		ProjectViewSelect(model::NodePtrs());
		WindowTriggerMenu(wxID_PASTE);
		ASSERT_EQUALS(ProjectViewCount(), 3 + files.size());
		ASSERT_EQUALS(getRoot()->getChildren().size(), 2 + files.size());
		Undo();
	}
	{
		StartTest("Paste from main menu (root selected, files in clipboard)");
        ProjectViewSelect({ getRoot() });
		WindowTriggerMenu(wxID_PASTE);
		ASSERT_EQUALS(ProjectViewCount(), 3 + +files.size());
		ASSERT_EQUALS(getRoot()->getChildren().size(), 2 + +files.size());
		Undo();
	}
	{
		StartTest("Paste from main menu (folder selected, files in clipboard)");
		model::FolderPtr folder = ProjectViewAddFolder("FOLDER");
		ASSERT_EQUALS(ProjectViewCount(), 4);
        ProjectViewSelect({ folder });
		WindowTriggerMenu(wxID_PASTE);
		ASSERT_EQUALS(ProjectViewCount(), 4 + +files.size());
		ASSERT_EQUALS(folder->getChildren().size(), + files.size());
		Undo(2);
	}
}


} // namespace
