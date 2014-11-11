// Copyright 2014 Eric Raijmakers.
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

#include "TestNewProject.h"

#include "DialogNewProject.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestNewProject::testEmptyFolder()
{
    StartTestSuite();
    ConfigOverruleString openwizard(Config::sPathDefaultNewProjectType, model::DefaultNewProjectWizardStart_toString(model::DefaultNewProjectWizardStartFiles));
    WindowTriggerMenu(wxID_NEW);
    WaitForIdle();
    ASSERT(!gui::Window::get().IsActive());
    ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageStart());
    gui::DialogNewProject::get().pressButtonFolder();
    WaitForIdle();
    ASSERT(gui::DialogNewProject::get().isNextEnabled());
    ASSERT(gui::DialogNewProject::get().isNextEnabled());
    gui::DialogNewProject::get().pressNext();
    WaitForIdle();
    ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageFolder());
    ASSERT(!gui::DialogNewProject::get().isNextEnabled());
    RandomTempDirPtr tempDir = RandomTempDir::generate();
    gui::Dialog::get().setDir(tempDir->getFileName().GetFullPath());
    gui::DialogNewProject::get().pressBrowseFolder();
    WaitForIdle();
    ASSERT(!gui::DialogNewProject::get().isNextEnabled());
    WaitForIdle();
    gui::DialogNewProject::get().pressCancel();
    WaitForIdle();
    ASSERT(!gui::Window::get().isProjectOpened());
    ASSERT_EQUALS(Config::ReadEnum<model::DefaultNewProjectWizardStart>(Config::sPathDefaultNewProjectType), model::DefaultNewProjectWizardStartFiles);
    ASSERT(!gui::TimelinesView::get().hasTimeline());
}

void TestNewProject::testFolder()
{
    StartTestSuite();
    ConfigOverruleString openwizard(Config::sPathDefaultNewProjectType, model::DefaultNewProjectWizardStart_toString(model::DefaultNewProjectWizardStartFolder));
    auto test = [](wxString foldername, FrameRate expectedFrameRate, int expectedSampleRate, int expectedChannels)
    {
        StartTest(foldername);
        WindowTriggerMenu(wxID_NEW);
        WaitForIdle();
        ASSERT(!gui::Window::get().IsActive());
        ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageStart());
        ASSERT(gui::DialogNewProject::get().isNextEnabled());
        gui::DialogNewProject::get().pressNext();
        WaitForIdle();
        ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageFolder());
        ASSERT(!gui::DialogNewProject::get().isNextEnabled());
        wxFileName input = getTestFilesPath(foldername);
        model::IPaths paths = getListOfInputPaths(input);
        gui::Dialog::get().setDir(input.GetFullPath());
        gui::DialogNewProject::get().pressBrowseFolder();
        WaitForIdle();
        ASSERT(gui::DialogNewProject::get().isNextEnabled());
        gui::DialogNewProject::get().pressNext();
        WaitForIdle();
        ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageProperties());
        WaitForIdle();
        gui::DialogNewProject::get().pressFinish();
        WaitForIdle();
        ASSERT(gui::Window::get().isProjectOpened());
        ASSERT(gui::TimelinesView::get().hasTimeline());
        ASSERT_EQUALS(VideoTrack(0)->getClips().size(), paths.size());
        ASSERT_EQUALS(AudioTrack(0)->getClips().size(), paths.size());
        ASSERT_EQUALS(model::Properties::get().getFrameRate(), expectedFrameRate);
        ASSERT_EQUALS(model::Properties::get().getAudioSampleRate(), expectedSampleRate);
        ASSERT_EQUALS(model::Properties::get().getAudioNumberOfChannels(), expectedChannels);
        model::Project::get().Modify(false); // Avoid save yes/no dialog.
        WindowTriggerMenu(wxID_CLOSE);
        WaitForIdle();
    };
    // Framerate set to most common framerate in folder
    test("newproject_folder1", FrameRate::s25p, 44100, 2); 
    test("newproject_folder2", FrameRate::s24p, 44100, 1);
    ASSERT_EQUALS(Config::ReadEnum<model::DefaultNewProjectWizardStart>(Config::sPathDefaultNewProjectType), model::DefaultNewProjectWizardStartFolder);
}

void TestNewProject::testNoUsableFiles()
{
    StartTestSuite();
    ConfigOverruleString openwizard(Config::sPathDefaultNewProjectType, model::DefaultNewProjectWizardStart_toString(model::DefaultNewProjectWizardStartFolder));
    WindowTriggerMenu(wxID_NEW);
    WaitForIdle();
    ASSERT(!gui::Window::get().IsActive());
    ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageStart());
    gui::DialogNewProject::get().pressButtonFiles();
    WaitForIdle();
    ASSERT(gui::DialogNewProject::get().isNextEnabled());
    gui::DialogNewProject::get().pressNext();
    WaitForIdle();
    ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageFiles());
    ASSERT(!gui::DialogNewProject::get().isNextEnabled());
    wxStrings files;
    gui::Dialog::get().setFiles(files);
    gui::DialogNewProject::get().pressBrowseFiles();
    WaitForIdle();
    ASSERT(!gui::DialogNewProject::get().isNextEnabled());
    WaitForIdle();
    gui::DialogNewProject::get().pressCancel();
    WaitForIdle();
    ASSERT(!gui::Window::get().isProjectOpened());
    ASSERT(!gui::TimelinesView::get().hasTimeline());
    ASSERT_EQUALS(Config::ReadEnum<model::DefaultNewProjectWizardStart>(Config::sPathDefaultNewProjectType), model::DefaultNewProjectWizardStartFolder);
}

void TestNewProject::testFiles()
{
    StartTestSuite();
    ConfigOverruleString openwizard(Config::sPathDefaultNewProjectType, model::DefaultNewProjectWizardStart_toString(model::DefaultNewProjectWizardStartFiles));
    WindowTriggerMenu(wxID_NEW);
    WaitForIdle();
    ASSERT(!gui::Window::get().IsActive());
    ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageStart());
    WaitForIdle();
    ASSERT(gui::DialogNewProject::get().isNextEnabled());
    gui::DialogNewProject::get().pressNext();
    WaitForIdle();
    ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageFiles());
    ASSERT(!gui::DialogNewProject::get().isNextEnabled());
    wxStrings files;
    for ( model::IPathPtr path : getListOfInputPaths() )
    {
        files.push_back(path->getPath().GetFullPath());
    }
    gui::Dialog::get().setFiles(files);
    gui::DialogNewProject::get().pressBrowseFiles();
    WaitForIdle();
    ASSERT(gui::DialogNewProject::get().isNextEnabled());
    gui::DialogNewProject::get().pressNext();
    WaitForIdle();
    ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageProperties());
    WaitForIdle();
    gui::DialogNewProject::get().pressFinish();
    WaitForIdle();
    ASSERT(gui::Window::get().isProjectOpened());
    ASSERT(gui::TimelinesView::get().hasTimeline());
    ASSERT_EQUALS(VideoTrack(0)->getClips().size(), files.size());
    ASSERT_EQUALS(AudioTrack(0)->getClips().size(), files.size());
    ASSERT_EQUALS(model::Properties::get().getFrameRate(), FrameRate::s24p);
    ASSERT_EQUALS(model::Properties::get().getAudioSampleRate(), 44100);
    ASSERT_EQUALS(model::Properties::get().getAudioNumberOfChannels(), 1);
    ASSERT_EQUALS(Config::ReadEnum<model::DefaultNewProjectWizardStart>(Config::sPathDefaultNewProjectType), model::DefaultNewProjectWizardStartFiles);
}

void TestNewProject::testBlank()
{
    StartTestSuite();
    ConfigOverruleString openwizard(Config::sPathDefaultNewProjectType, model::DefaultNewProjectWizardStart_toString(model::DefaultNewProjectWizardStartFolder));
    WindowTriggerMenu(wxID_NEW);
    WaitForIdle();
    ASSERT(!gui::Window::get().IsActive());
    ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageStart());
    gui::DialogNewProject::get().pressButtonBlank();
    WaitForIdle();
    ASSERT(gui::DialogNewProject::get().isNextEnabled());
    gui::DialogNewProject::get().pressNext();
    WaitForIdle();
    ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageProperties());
    WaitForIdle();
    gui::DialogNewProject::get().pressFinish();
    WaitForIdle();
    ASSERT(gui::Window::get().isProjectOpened());
    ASSERT(!gui::TimelinesView::get().hasTimeline());
    ASSERT_EQUALS(Config::ReadEnum<model::DefaultNewProjectWizardStart>(Config::sPathDefaultNewProjectType), model::DefaultNewProjectWizardStartBlank);
}

void TestNewProject::testCancel()
{
    StartTestSuite();
    ConfigOverruleString openwizard(Config::sPathDefaultNewProjectType, model::DefaultNewProjectWizardStart_toString(model::DefaultNewProjectWizardStartBlank));
    WindowTriggerMenu(wxID_NEW);
    WaitForIdle();
    ASSERT(!gui::Window::get().IsActive());
    ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageStart());
    ASSERT(gui::DialogNewProject::get().isNextEnabled());
    gui::DialogNewProject::get().pressNext();
    WaitForIdle();
    ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageProperties());
    WaitForIdle();
    gui::DialogNewProject::get().pressCancel();
    WaitForIdle();
    ASSERT(!gui::Window::get().isProjectOpened());
    ASSERT(!gui::TimelinesView::get().hasTimeline());
    ASSERT_EQUALS(Config::ReadEnum<model::DefaultNewProjectWizardStart>(Config::sPathDefaultNewProjectType), model::DefaultNewProjectWizardStartBlank);
}

} // namespace