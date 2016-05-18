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

#include "TestSavingAndLoading.h"

namespace test {

const wxString sCurrent("current.vid");
const wxString sFolder("folder");

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

    ConfigOverrule<wxString> overruleFrameRate(Config::sPathVideoDefaultFrameRate, FrameRate::s24p.toString());
    ConfigOverrule<long> overruleChannels(Config::sPathAudioDefaultNumberOfChannels, 2);
    ConfigOverrule<long> overruleSampleRate(Config::sPathAudioDefaultSampleRate, 44100);

    StartTest("Create saved document");

    ExpectExecutedWork expectation(14, true);
    FixtureProject mProjectFixture(true);
    mProjectFixture.init();
    // Wait until audio peaks generated. Otherwise, not all save files have the same contents.
    // The later save files may have more entries in the meta data cache.
    expectation.wait();

    StartTest("Add non auto folder to project view");
    model::FolderPtr folder1 = ProjectViewAddFolder( sFolder );

    StartTest("Add still image to project view");
    ProjectViewAddFiles({ getStillImagePath() }, folder1);

    StartTest("Add video clips to sequence");
    ExtendSequenceWithRepeatedClips(getSequence(), getListOfInputPaths(), 1);

    // Ensure each transition type is saved once
    int number = 6;
    for ( model::TransitionPtr t : model::video::VideoTransitionFactory::get().getAllPossibleTransitions() )
    {
        for (model::TransitionType direction : { model::TransitionTypeFadeIn, model::TransitionTypeFadeInFromPrevious, model::TransitionTypeFadeOut, model::TransitionTypeFadeOutToNext })
        {
            if (t->supports(direction))
            {
                std::ostringstream os; os  << "Add " << t->getDescription() << " " << model::TransitionType_toString(direction);
                StartTest(os.str().c_str());

                if (static_cast<int>(AudioTrack(0)->getClips().size()) < number + 2) // Use #clips in audiotrack since the #clips in the videotrack includes the transitions
                {
                    // Ensure that the next transition can be positioned
                    ExtendSequenceWithRepeatedClips(getSequence(), getListOfInputPaths(), 1);
                }

                util::thread::RunInMainAndWait([t, direction, number]()
                {
                    gui::timeline::cmd::createTransition(getSequence(), VideoClip(0, number), direction, make_cloned<model::Transition>(t));
                });
                number += (direction == model::TransitionTypeFadeOutToNext) ? 3 : 2;
            }
        }
    }

    StartTest("Add still image to sequence");
    ExtendSequenceWithStillImage(getSequence()); // Ensure that there is a still image in the timeline

    StartTest("Add intervals to timeline");
    ToggleInterval(HCenter(VideoClip(0,0)), RightPixel(VideoClip(0,2)) - 10);
    ToggleInterval(HCenter(VideoClip(0,1)), HCenter(VideoClip(0,2)));

    StartTest("Add empty clip to timeline");
    TimelineLeftClick(Center(VideoClip(0,1)));
    TimelineKeyPress(WXK_DELETE);

    StartTest("Change default video key frame");
    TimelineSelectClips({ VideoClip(0,3) });
    SetValue(DetailsClipView()->getRotationSlider(), -4500);
    ASSERT(DefaultKeyFrame(VideoClip(0, 3)).Rotation(rational64{ -45, 1 }));

    StartTest("Add and change non-default video key frames");
    TimelineSelectClips({ VideoClip(0,4) });
    TimelinePositionCursor(HCenter(VideoClip(0, 4)));
    ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
    SetValue(DetailsClipView()->getScalingSlider(), 7000);
    TimelinePositionCursor(HCenter(VideoClip(0, 4)) - 20);
    ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
    SetValue(DetailsClipView()->getOpacitySlider(), 128);
    TimelinePositionCursor(HCenter(VideoClip(0, 4)) + 30);
    ButtonTriggerPressed(DetailsClipView()->getVideoKeyFramesAddButton());
    SetValue(DetailsClipView()->getPositionXSlider(), -200);
    SetValue(DetailsClipView()->getPositionYSlider(), -111);

    StartTest("Zoom");
    TimelineKeyPressN(4,'='); // Zoom in until factor is 1:1

    StartTest("Scroll");
    util::thread::RunInMainAndWait([]()
    {
        getTimeline().getScrolling().align(getSequence()->getLength() / 2, getTimeline().GetSize().x / 2);
    });

    StartTest("Move cursor position");
    TimelinePositionCursor(getTimeline().getZoom().ptsToPixels(getSequence()->getLength() / 2));



    //////////////////////////////////////////////////////////////////////////

    // This project files is saved IN the source tree. This is required for saving
    // with relative path names. This avoids problems when running this test from a
    // repository in a different path.
    wxFileName referenceDirName(getTestPath());
    referenceDirName.AppendDir("saved_projects");
    mProjectFixture.destroy();
    std::pair<RandomTempDirPtr, wxFileName> tempDir_fileName = SaveProjectAndClose(boost::make_shared<RandomTempDir>(referenceDirName), "_new");

    //////////////////////////////////////////////////////////////////////////

    {
        StartTest("Compare generated document with reference document");
        wxFileName referenceFileName(referenceDirName);
        referenceFileName.SetFullName(sCurrent);
        if (!getSavedFileContents(tempDir_fileName.second).IsSameAs(getSavedFileContents(referenceFileName)))
        {
            OpenFileExplorer(referenceDirName);
            FATAL("File contents are not equal");
        }
    }

    //////////////////////////////////////////////////////////////////////////

    checkDocument(tempDir_fileName.second);

    bool ok = wxRemoveFile(tempDir_fileName.second.GetLongPath());
    ASSERT(ok)(tempDir_fileName.second.GetLongPath());
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
            wxRegEx reVersion("[[:digit:]]+\\.[[:digit:]]+\\.[[:digit:]]+_([[:digit:]]+)\\." + model::Project::sFileExtension);
            ASSERT(reVersion.IsValid());
            ASSERT_EQUALS(reVersion.GetMatchCount(),2);
            bool match = reVersion.Matches(filename);
            ASSERT(match);
            long revision = wxAtoi(reVersion.GetMatch(filename,1));
            if (wxPlatformInfo::Get().GetOperatingSystemId() == wxOS_UNIX_LINUX && revision < 1602) { continue; } // Older saved projects contain absolute windows paths which are unreadible.
			StartTest(filename.c_str());
            wxFileName vidFileName(referenceDirName);
            vidFileName.SetFullName(filename);
            checkDocument(vidFileName);
        }
    }
}

void TestSavingAndLoading::testBackupBeforeSave()
{
    StartTestSuite();

    ExpectExecutedWork expectation(14, true);
    FixtureProject mProjectFixture(true);
    mProjectFixture.init();
    // Wait until audio peaks generated. Otherwise, not all save files have the same contents.
    // The later save files may have more entries in the meta data cache.
    expectation.wait();

    StartTest("Create temp dir and existing file");
    RandomTempDirPtr tempDirProject = RandomTempDir::generate();
    wxFileName existingFile = generateSaveFileName(tempDirProject->getFileName());
    DirAndFile tempDir_fileName = SaveProject(tempDirProject);
    wxString ExpectedContents = getSavedFileContents(existingFile);

    auto ASSERT_FILE_CREATED = [&](wxFileName prefix, int count)
    {
        wxFileName filename = model::Project::createBackupFileName(prefix,count);
        ASSERT(wxFile::Exists(filename.GetLongPath()));
        ASSERT(getSavedFileContents(filename).IsSameAs(ExpectedContents));
    };

    //////////////////////////////////////////////////////////////////////////

    StartTest("Save again multiple times and check that backup files are generated");
    for (int count = 0; count < 60; ++count)
    {
        ASSERT(!model::Project::createBackupFileName(existingFile,count).Exists())(count);
        tempDir_fileName = SaveProject(tempDirProject);
        ASSERT(model::Project::createBackupFileName(existingFile,count).Exists())(count);
        ASSERT_FILE_CREATED(existingFile,count);

        for (int j = 0; j < count - Config::get().read<long>(Config::sPathProjectBackupBeforeSaveMaximum); ++j)
        {
            ASSERT(!model::Project::createBackupFileName(existingFile,j).Exists())(count)(j)(tempDir_fileName);
        }
    }
    mProjectFixture.destroy();
    CloseProjectAndAvoidSaveDialog();
}

void TestSavingAndLoading::testRevert()
{
    StartTestSuite();
    FixtureProject mProjectFixture(true);
    mProjectFixture.init();
    StartTest("Create temp dir and existing file");
    mProjectFixture.destroy();
    std::pair<RandomTempDirPtr, wxFileName> tempDir_fileName = SaveProjectAndClose(RandomTempDir::generate(), "");
    OpenProject(tempDir_fileName.second.GetLongPath());

    TimelineTrimLeft(VideoClip(0,3), 10, false);
    TimelineDeleteClip(VideoClip(0,5));
    TimelineDrag(From(Center(VideoClip(0, 0))).To(Center(VideoClip(0, 7)) + wxPoint(150, 0)));

    OpenProjectWaiter waitForOpenedProject;
    WindowTriggerMenu(wxID_REVERT);
    WaitUntilDialogOpen(true);
    wxUIActionSimulator().Char(WXK_RETURN);
    WaitUntilDialogOpen(false);
    waitForOpenedProject.wait();

    ASSERT_VIDEOTRACK0(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip)(VideoClip);
    ASSERT_AUDIOTRACK0(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip)(AudioClip);

    for (size_t i = 0; i < mProjectFixture.InputFiles.size(); ++i)
    {
        ASSERT_EQUALS(VideoClip(0, i)->getLeftPts(), mProjectFixture.mOriginalPtsOfVideoClip[i]);
        ASSERT_EQUALS(VideoClip(0, i)->getLength(), mProjectFixture.mOriginalLengthOfVideoClip[i]);
        ASSERT_EQUALS(AudioClip(0, i)->getLeftPts(), mProjectFixture.mOriginalPtsOfAudioClip[i]);
        ASSERT_EQUALS(AudioClip(0, i)->getLength(), mProjectFixture.mOriginalLengthOfAudioClip[i]);
    }
}

///////////////////////////d///////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void TestSavingAndLoading::checkDocument(wxFileName path)
{
    StartTest(path.GetName() + ": "  + "Load document");
    OpenProject(path.GetLongPath());

    // Checks on loaded document
    // Must wait for Idle twice, since the timeline class uses
    // two Idle events to start its initialization
    // See the use of Timeline::mExecuteOnIdle
    WaitForIdle;
    WaitForIdle;
    StartTest(path.GetName() + ": " + "Cursor position");
    util::thread::RunInMainAndWait([path]()
    {
        ASSERT_EQUALS(getTimeline().getCursor().getLogicalPosition(), getSequence()->getLength() / 2);
        ASSERT_EQUALS(getTimeline().getScrolling().getCenterPts(), getSequence()->getLength() / 2);
    });
    // Actions on loaded document
    util::thread::RunInMainAndWait([]()
    {
        // First move to the left so that all the move actions succeed
        getTimeline().getScrolling().align(0,0);
    });
    util::thread::RunInMainAndWait([path]()
    {
        // Known bug at some point: mLastModified not known for a recently opened file (in the project view).
        // The method geteLastModified was accessed when the date column comes into view.
        StartTest(path.GetName() + ": " + "Open folder");
        ProjectViewExpandInput();
    });
    util::thread::RunInMainAndWait([]()
    {
        gui::ProjectView::get().scrollToRight();
    });
    {
        StartTest(path.GetName() + ": " + "Trim clip"); // Known bug at some point: a crash due to improper initialization of File class members upon loading (mNumberOfFrames not initialized)
        TimelineTrimLeft(VideoClip(0,1),20);
        Undo();
    }
    {
        StartTest(path.GetName() + ": " + "Enlarge sequence"); // Known bug at some point: enlarging the sequence did not cause an update of the timeline virtual size due to missing event binding
        wxSize paneSize = getTimeline().GetVirtualSize();
        wxSize size = getTimeline().getSequenceView().getSize();
        ExtendSequenceWithRepeatedClips(getSequence(), getListOfInputPaths(), 1);
        ASSERT_DIFFERS(getTimeline().getSequenceView().getSize(), size);
        ASSERT_DIFFERS(getTimeline().GetVirtualSize(), paneSize);
    }
    {
        StartTest(path.GetName() + ": " + "Scrub");
        Scrub(HCenter(VideoClip(0,3)),HCenter(VideoClip(0,3)) + 20);
    }
    {
        StartTest(path.GetName() + ": " + "Open render settings"); // Known bug at some point: loading the project went ok, but when opening the render dialog a crash occurred.
        WindowTriggerMenu(ID_RENDERSETTINGS);
        WaitUntilDialogOpen(true);
        ButtonTriggerPressed(gui::DialogRenderSettings::get().getCancelButton());
        WaitUntilDialogOpen(false);
    }
    CloseProjectAndAvoidSaveDialog();
}

} // namespace
