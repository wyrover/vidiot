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

#include "CreateTransitionHelper.h"
#include "FixtureProject.h"
#include "HelperApplication.h"
#include "HelperFileSystem.h"
#include "HelperProject.h"
#include "HelperThread.h"
#include "HelperTimeline.h"
#include "HelperTimelineTrim.h"
#include "HelperTimelinesView.h"
#include "HelperWindow.h"
#include "Project.h"
#include "Scrolling.h"
#include "Sequence.h"
#include "Timeline.h"
#include "VideoTransitionFactory.h"
#include "Window.h"
#include "Zoom.h"

namespace test {

const wxString sCurrent("current.vid");

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestSavingAndLoading::setUp()
{
}

void TestSavingAndLoading::tearDown()
{
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestSavingAndLoading::testSaveAndLoad()
{
    StartTestSuite();

    StartTest("Create saved document");

    FixtureProject mProjectFixture(true);
    mProjectFixture.init();

    StartTest("Add non auto folder to project view");
    model::FolderPtr folder1 = addFolder( "folder" );

    StartTest("Add still image to project view");
    addFiles( boost::assign::list_of(getStillImagePath()), folder1 );

    StartTest("Add video clips to sequence");
    extendSequenceWithRepeatedClips(getSequence(), getListOfInputFiles(), 2);

    StartTest("Add still image to sequence");
    extendSequenceWithStillImage(getSequence()); // Ensure that there is a still image in the timeline

    // Ensure each transition type is saved once
    int number = 3;
    for ( model::TransitionDescription t : model::video::VideoTransitionFactory::get().getAllPossibleTransitions() )
    {
        StartTest("Add transition (" + t.first + "," + t.second + ") to sequence");
        RunInMainAndWait([t,number]() { gui::timeline::command::createTransition(getSequence(), VideoClip(0,number),model::TransitionTypeIn, model::video::VideoTransitionFactory::get().getTransition(t)); });
        number += 2; // +2 because the transition was added inbetween
    }

    StartTest("Add intervals to timeline");
    ToggleInterval(HCenter(VideoClip(0,0)), RightPixel(VideoClip(0,2)) - 10);
    ToggleInterval(HCenter(VideoClip(0,1)), HCenter(VideoClip(0,2)));

    // todo all codecs and parameters

    StartTest("Add empty clip to timeline");
    Click(Center(VideoClip(0,1)));
    Type(WXK_DELETE);

    StartTest("Zoom");
    TypeN(4,'='); // Zoom in until factor is 1:1

    StartTest("Scroll");
    RunInMainAndWait([]()
    {
        getTimeline().getScrolling().align(getSequence()->getLength() / 2, getTimeline().GetSize().x / 2);
    });

    StartTest("Move cursor position");
    PositionCursor(getTimeline().getZoom().ptsToPixels(getSequence()->getLength() / 2));

    //////////////////////////////////////////////////////////////////////////

    std::pair<RandomTempDirPtr, wxFileName> tempDir_fileName = SaveProjectAndClose();

    //////////////////////////////////////////////////////////////////////////

    {
        StartTest("Compare generated document with reference document");
        wxFileName referenceDirName(getTestPath());
        referenceDirName.AppendDir("saved_projects");
        wxFileName referenceFileName(referenceDirName);
        referenceFileName.SetFullName(sCurrent);
        if (!getFileContents(tempDir_fileName.second).IsSameAs(getFileContents(referenceFileName)))
        {
            wxFileName newCurrentFileName(referenceDirName);
            newCurrentFileName.SetFullName(tempDir_fileName.second.GetName() + "_new.vid");
            bool ok = wxCopyFile(tempDir_fileName.second.GetFullPath(), newCurrentFileName.GetFullPath());
            RunInMainAndWait([referenceDirName]()
            {
                wxString cmd;
                cmd << "explorer " << referenceDirName.GetFullPath();
                ::wxExecute( cmd, wxEXEC_ASYNC, NULL);
            });
            ASSERT(ok);
            FATAL("File contents are not equal");
        }
    }

    //////////////////////////////////////////////////////////////////////////

    checkDocument(tempDir_fileName.second.GetFullPath());

    mProjectFixture.destroy();
}

void TestSavingAndLoading::testLoadOldVersions()
{
    StartTestSuite();

    wxFileName referenceDirName(getTestPath());
    referenceDirName.AppendDir("saved_projects");
    wxDir dir( referenceDirName.GetLongPath() );
    wxString filename;
    for (bool cont = dir.GetFirst(&filename,wxEmptyString,wxDIR_FILES); cont; cont = dir.GetNext(&filename))
    {
        if (!filename.IsSameAs(sCurrent))
        {
            wxFileName vidFileName(referenceDirName);
            vidFileName.SetFullName(filename);
            checkDocument(vidFileName.GetFullPath());
        }
    }
}

void TestSavingAndLoading::testBackupBeforeSave()
{
    StartTestSuite();

    FixtureProject mProjectFixture(true);
    mProjectFixture.init();

    StartTest("Create temp dir and existing file");
    RandomTempDirPtr tempDirProject = RandomTempDir::generate();
    wxFileName existingFile = generateSaveFileName(tempDirProject->getFileName());
    DirAndFile tempDir_fileName = SaveProject(tempDirProject);
    wxString ExpectedContents = getFileContents (existingFile);

    auto ASSERT_FILE_CREATED = [ExpectedContents](wxFileName prefix, int count)
    {
        wxFileName filename = model::Project::createBackupFileName(prefix,count);
        ASSERT(wxFile::Exists(filename.GetFullPath()));
        ASSERT(getFileContents(filename).IsSameAs(ExpectedContents));
    };

    //////////////////////////////////////////////////////////////////////////

    StartTest("Save again multiple times and check that backup files are generated");
    std::vector<bool> isPresent = std::vector<bool>(20, false); // Indicates which files should be present/not
    for (int count = 0; count < 60; ++count)
    {
        ASSERT(!model::Project::createBackupFileName(existingFile,count).Exists());
        tempDir_fileName = SaveProject(tempDirProject);
        ASSERT(model::Project::createBackupFileName(existingFile,count).Exists());
        ASSERT_FILE_CREATED(existingFile,count);

        for (int j = 0; j < count - Config::ReadLong(Config::sPathBackupBeforeSaveMaximum); ++j)
        {
            ASSERT(!model::Project::createBackupFileName(existingFile,j).Exists());
        }
    }
    triggerMenu(wxID_CLOSE);
    waitForIdle();
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void TestSavingAndLoading::checkDocument(wxString path)
{
    StartTest("Load document");
    RunInMainAndWait([path]()
    {
        gui::Window::get().GetDocumentManager()->CreateDocument(path,wxDOC_SILENT);
    });

    // Checks on loaded document
    waitForIdle();
    {
        StartTest("Cursor position");
        ASSERT_EQUALS(getTimeline().getCursor().getLogicalPosition(), getSequence()->getLength() / 2);
    }
    {
        StartTest("Scroll offset");
        ASSERT_EQUALS(getTimeline().getScrolling().getCenterPts(), getSequence()->getLength() / 2);
    }

    // Actions on loaded document
    RunInMainAndWait([]()
    {
        // First move to the left so that all the move actions succeed
        getTimeline().getScrolling().align(0,0);
    });
    {
        StartTest("Trim clip"); // Known bug at some point: a crash due to improper initialization of File class members upon loading (mNumberOfFrames not initialized)
        TrimLeft(VideoClip(0,1),20);
        Undo();
    }
    {
        StartTest("Enlarge sequence"); // Known bug at some point: enlarging the sequence did not cause an update of the timeline virtual size due to missing event binding
        wxSize paneSize = getTimeline().GetVirtualSize();
        wxSize size = getTimeline().getSequenceView().getSize();
        extendSequenceWithRepeatedClips(getSequence(), getListOfInputFiles(), 1);
        ASSERT_DIFFERS(getTimeline().getSequenceView().getSize(), size);
        ASSERT_DIFFERS(getTimeline().GetVirtualSize(), paneSize);
    }
    {
        StartTest("Scrub");
        Scrub(HCenter(VideoClip(0,3)),HCenter(VideoClip(0,3)) + 20);
    }
    {
        StartTest("Open render settings"); // Known bug at some point: loading the project went ok, but when opening the render dialog a crash occurred.
        WaitForTimelineToLoseFocus w;
        triggerMenu(ID_RENDERSETTINGS);
        w.wait();
        wxUIActionSimulator().Char(WXK_ESCAPE);
    }
    {
        StartTest("Close");
        model::Project::get().Modify(false); // Avoid 'save?' dialog
        triggerMenu(wxID_CLOSE);
    }
}

} // namespace