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

#include "TestNewProject.h"

#include "DialogNewProject.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestNewProject::testEmptyFolder()
{
    StartTestSuite();
    RandomTempDirPtr tempDir = RandomTempDir::generate();
    gui::Dialog::get().setDir(tempDir->getFileName().GetFullPath());
    ASSERT(!util::thread::RunInMainReturning<bool>([] { return gui::Window::get().isProjectOpened(); } ));
    ConfigOverrule<wxString> openwizard(Config::sPathProjectDefaultNewProjectType, model::DefaultNewProjectWizardStart_toString(model::DefaultNewProjectWizardStartFiles));
    WindowTriggerMenu(wxID_NEW);
    WaitUntilMainWindowActive(false);
    ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageStart());
    gui::DialogNewProject::get().pressButtonFolder();
    WaitForIdle;
    ASSERT(gui::DialogNewProject::get().isNextEnabled());
    gui::DialogNewProject::get().pressNext();
    WaitForIdle;
    ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageFolder());
    ASSERT(!gui::DialogNewProject::get().isNextEnabled());
    WaitForIdle;
    gui::DialogNewProject::get().pressCancel();
    WaitUntilMainWindowActive(true);
    ASSERT(!util::thread::RunInMainReturning<bool>([] { return gui::Window::get().isProjectOpened(); } ));
    ASSERT_CONFIG_ENUM(model::DefaultNewProjectWizardStart, Config::sPathProjectDefaultNewProjectType, model::DefaultNewProjectWizardStartFiles);
    ASSERT(!gui::TimelinesView::get().hasTimeline());
}

void TestNewProject::testFolder()
{
    StartTestSuite();
    ASSERT(!util::thread::RunInMainReturning<bool>([] { return gui::Window::get().isProjectOpened(); } ));
    ConfigOverrule<wxString> openwizard(Config::sPathProjectDefaultNewProjectType, model::DefaultNewProjectWizardStart_toString(model::DefaultNewProjectWizardStartFolder));
    auto test = [](wxString foldername, FrameRate expectedFrameRate, int expectedSampleRate, int expectedChannels)
    {
        StartTest(foldername);

        WindowTriggerMenu(wxID_NEW);
        WaitUntilMainWindowActive(false);

        wxFileName input = getTestFilesPath(foldername);
        model::IPaths paths = getListOfInputPaths(input);
        gui::Dialog::get().setDir(input.GetFullPath());

        ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageStart());
        ASSERT(gui::DialogNewProject::get().isNextEnabled());
        gui::DialogNewProject::get().pressNext();
        WaitForIdle;
        ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageFolder());
        ASSERT(gui::DialogNewProject::get().isNextEnabled());
        gui::DialogNewProject::get().pressNext();
        WaitForIdle;
        ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageProperties());
        WaitForIdle;
        gui::DialogNewProject::get().pressFinish();
        WaitUntilMainWindowActive(true);
        ASSERT(util::thread::RunInMainReturning<bool>([] { return gui::Window::get().isProjectOpened(); } ));
        ASSERT(gui::TimelinesView::get().hasTimeline());
        ASSERT_VIDEOTRACK0SIZE(paths.size());
        ASSERT_AUDIOTRACK0SIZE(paths.size());
        ASSERT_EQUALS(model::Properties::get().getFrameRate(), expectedFrameRate);
        ASSERT_EQUALS(model::Properties::get().getAudioSampleRate(), expectedSampleRate);
        ASSERT_EQUALS(model::Properties::get().getAudioNumberOfChannels(), expectedChannels);
        model::Project::get().Modify(false); // Avoid save yes/no dialog.
        WindowTriggerMenu(wxID_CLOSE);
        WaitForIdle;
    };
    // Framerate set to most common framerate in folder
    test("newproject_folder1", FrameRate::s25p, 44100, 2);
    test("newproject_folder2", FrameRate::s24p, 44100, 1);
    ASSERT_CONFIG_ENUM(model::DefaultNewProjectWizardStart, Config::sPathProjectDefaultNewProjectType, model::DefaultNewProjectWizardStartFolder);
}

void TestNewProject::testNoUsableFiles()
{
    StartTestSuite();
    ASSERT(!util::thread::RunInMainReturning<bool>([] { return gui::Window::get().isProjectOpened(); } ));
    ConfigOverrule<wxString> openwizard(Config::sPathProjectDefaultNewProjectType, model::DefaultNewProjectWizardStart_toString(model::DefaultNewProjectWizardStartFolder));
    WindowTriggerMenu(wxID_NEW);
    WaitUntilMainWindowActive(false);
    wxStrings files;
    gui::Dialog::get().setFiles(files);
    ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageStart());
    gui::DialogNewProject::get().pressButtonFiles();
    WaitForIdle;
    ASSERT(gui::DialogNewProject::get().isNextEnabled());
    gui::DialogNewProject::get().pressNext();
    WaitForIdle;
    ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageFiles());
    ASSERT(!gui::DialogNewProject::get().isNextEnabled());
    WaitForIdle;
    gui::DialogNewProject::get().pressCancel();
    WaitUntilMainWindowActive(true);
    ASSERT(!util::thread::RunInMainReturning<bool>([] { return gui::Window::get().isProjectOpened(); } ));
    ASSERT(!gui::TimelinesView::get().hasTimeline());
    ASSERT_CONFIG_ENUM(model::DefaultNewProjectWizardStart, Config::sPathProjectDefaultNewProjectType, model::DefaultNewProjectWizardStartFolder);
}

void TestNewProject::testFiles()
{
    StartTestSuite();
    ASSERT(!util::thread::RunInMainReturning<bool>([] { return gui::Window::get().isProjectOpened(); } ));
    ConfigOverrule<wxString> openwizard(Config::sPathProjectDefaultNewProjectType, model::DefaultNewProjectWizardStart_toString(model::DefaultNewProjectWizardStartFiles));
    WindowTriggerMenu(wxID_NEW);
    WaitUntilMainWindowActive(false);
    wxStrings files;
    for ( model::IPathPtr path : getListOfInputPaths() )
    {
        files.emplace_back(path->getPath().GetFullPath());
    }
    gui::Dialog::get().setFiles(files);
    ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageStart());
    WaitForIdle;
    ASSERT(gui::DialogNewProject::get().isNextEnabled());
    gui::DialogNewProject::get().pressNext();
    WaitForIdle;
    ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageFiles());
    ASSERT(gui::DialogNewProject::get().isNextEnabled());
    WaitForIdle;
    ASSERT(gui::DialogNewProject::get().isNextEnabled());
    gui::DialogNewProject::get().pressNext();
    WaitForIdle;
    ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageProperties());
    WaitForIdle;
    gui::DialogNewProject::get().pressFinish();
    WaitUntilMainWindowActive(true);
    ASSERT(util::thread::RunInMainReturning<bool>([] { return gui::Window::get().isProjectOpened(); } ));
    ASSERT(gui::TimelinesView::get().hasTimeline());
    ASSERT_VIDEOTRACK0SIZE(files.size());
    ASSERT_AUDIOTRACK0SIZE(files.size());
    ASSERT_EQUALS(model::Properties::get().getFrameRate(), FrameRate::s24p);
    ASSERT_EQUALS(model::Properties::get().getAudioSampleRate(), 44100);
    ASSERT_EQUALS(model::Properties::get().getAudioNumberOfChannels(), 1);
    ASSERT_CONFIG_ENUM(model::DefaultNewProjectWizardStart, Config::sPathProjectDefaultNewProjectType, model::DefaultNewProjectWizardStartFiles);
}

void TestNewProject::testBlank()
{
    StartTestSuite();
    ASSERT(!util::thread::RunInMainReturning<bool>([] { return gui::Window::get().isProjectOpened(); } ));
    ConfigOverrule<wxString> openwizard(Config::sPathProjectDefaultNewProjectType, model::DefaultNewProjectWizardStart_toString(model::DefaultNewProjectWizardStartFolder));
    WindowTriggerMenu(wxID_NEW);
    WaitUntilMainWindowActive(false);
    ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageStart());
    gui::DialogNewProject::get().pressButtonBlank();
    WaitForIdle;
    ASSERT(gui::DialogNewProject::get().isNextEnabled());
    gui::DialogNewProject::get().pressNext();
    WaitForIdle;
    ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageProperties());
    WaitForIdle;
    gui::DialogNewProject::get().pressFinish();
    WaitUntilMainWindowActive(true);
    ASSERT(util::thread::RunInMainReturning<bool>([] { return gui::Window::get().isProjectOpened(); } ));
    ASSERT(!gui::TimelinesView::get().hasTimeline());
    ASSERT_CONFIG_ENUM(model::DefaultNewProjectWizardStart, Config::sPathProjectDefaultNewProjectType, model::DefaultNewProjectWizardStartBlank);
}

void TestNewProject::testCancel()
{
    StartTestSuite();
    ASSERT(!util::thread::RunInMainReturning<bool>([] { return gui::Window::get().isProjectOpened(); } ));
    ConfigOverrule<wxString> openwizard(Config::sPathProjectDefaultNewProjectType, model::DefaultNewProjectWizardStart_toString(model::DefaultNewProjectWizardStartBlank));
    WindowTriggerMenu(wxID_NEW);
    WaitUntilMainWindowActive(false);
    ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageStart());
    ASSERT(gui::DialogNewProject::get().isNextEnabled());
    gui::DialogNewProject::get().pressNext();
    WaitForIdle;
    ASSERT_EQUALS(gui::DialogNewProject::get().GetCurrentPage(), gui::DialogNewProject::get().getPageProperties());
    WaitForIdle;
    gui::DialogNewProject::get().pressCancel();
    WaitUntilMainWindowActive(true);
    ASSERT(!util::thread::RunInMainReturning<bool>([] { return gui::Window::get().isProjectOpened(); } ));
    ASSERT(!gui::TimelinesView::get().hasTimeline());
    ASSERT_CONFIG_ENUM(model::DefaultNewProjectWizardStart, Config::sPathProjectDefaultNewProjectType, model::DefaultNewProjectWizardStartBlank);
}

} // namespace
