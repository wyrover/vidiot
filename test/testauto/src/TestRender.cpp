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

#include "TestRender.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestRender::setUp()
{
    mProjectFixture.init();
}

void TestRender::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestRender::testChangeRenderSettings()
{
    StartTestSuite();
    {
        StartTest("If cancel is pressed, nothing is changed.");
        model::render::RenderPtr original = GetCurrentRenderSettings();
        std::pair< RandomTempDirPtr, wxFileName > tempdir_and_filename = OpenDialogAndSetFilename();
        SetValue(dynamic_cast<wxSpinCtrl*>(gui::DialogRenderSettings::get().getVideoParam(model::render::BitRate)), 3999996);
        ButtonTriggerPressed(gui::DialogRenderSettings::get().getCancelButton());
        model::render::RenderPtr current = GetCurrentRenderSettings();
        ASSERT_EQUALS(*original,*current);
    }
    {
        StartTest("If apply is pressed, the sequence is changed (with a different video codec setting).");
        model::render::RenderPtr original = GetCurrentRenderSettings();
        std::pair< RandomTempDirPtr, wxFileName > tempdir_and_filename = OpenDialogAndSetFilename();
        SetValue(dynamic_cast<wxSpinCtrl*>(gui::DialogRenderSettings::get().getVideoParam(model::render::BitRate)), 399999);
        ButtonTriggerPressed(gui::DialogRenderSettings::get().getApplyButton());
        model::render::RenderPtr current = GetCurrentRenderSettings();
        ASSERT_DIFFERS(*original,*current);
        ButtonTriggerPressed(gui::DialogRenderSettings::get().getCancelButton());
    }
    {
        StartTest("If apply is pressed, the sequence is changed (with a different audio codec setting).");
        model::render::RenderPtr original = GetCurrentRenderSettings();
        std::pair< RandomTempDirPtr, wxFileName > tempdir_and_filename = OpenDialogAndSetFilename();
        SetValue(dynamic_cast<wxSpinCtrl*>(gui::DialogRenderSettings::get().getVideoParam(model::render::BitRate)), 300000);
        ButtonTriggerPressed(gui::DialogRenderSettings::get().getApplyButton());
        model::render::RenderPtr current = GetCurrentRenderSettings();
        ASSERT_DIFFERS(*original,*current);
        ButtonTriggerPressed(gui::DialogRenderSettings::get().getCancelButton());
    }
    {
        StartTest("If OK is pressed, the sequence is changed and the dialog is closed.");
        model::render::RenderPtr original = GetCurrentRenderSettings();
        std::pair< RandomTempDirPtr, wxFileName > tempdir_and_filename = OpenDialogAndSetFilename();
        SetValue(dynamic_cast<wxSpinCtrl*>(gui::DialogRenderSettings::get().getVideoParam(model::render::BitRate)), 200000);
        ButtonTriggerPressed(gui::DialogRenderSettings::get().getOkButton());
        model::render::RenderPtr current = GetCurrentRenderSettings();
        ASSERT_DIFFERS(*original,*current);
    }
}

void TestRender::testRenderingSplit()
{
    StartTestSuite();
    ConfigOverruleLong overrule(Config::sPathDebugMaxRenderLength, 1); // Only render 1s
    {
        StartTest("Render each part of the sequence separately.");
        ExpectExecutedWork expectation(3);
        TimelineLeftClick(Center(VideoClip(0,2)));
        TimelineKeyDown(wxMOD_CONTROL);
        TimelineLeftClick(Center(VideoClip(0,4))); // Exclude clip 3 deliberately: include empty clips in the list of intervals
        TimelineLeftClick(Center(VideoClip(0,5)));
        TimelineLeftClick(Center(VideoClip(0,6)));
        TimelineKeyUp(wxMOD_CONTROL);
        TimelineKeyPress(WXK_DELETE);
        model::render::RenderPtr original = GetCurrentRenderSettings();
        std::pair< RandomTempDirPtr, wxFileName > tempdir_and_filename = OpenDialogAndSetFilename();
        SetValue(gui::DialogRenderSettings::get().getRenderSeparationCheckBox(),true);
        ButtonTriggerPressed(gui::DialogRenderSettings::get().getRenderButton());
        expectation.wait();
        for (int i = 1; i <= 4; ++i)
        {
            wxFileName f(tempdir_and_filename.first->getFileName().GetLongPath(), wxString::Format("out_%d",i), "avi");
            ASSERT_IMPLIES(i <= 3, f.Exists());
            ASSERT_IMPLIES(i == 4, !f.Exists());
        }
    }
}

void TestRender::testRenderingCodecs()
{
    StartTestSuite();
    model::SequencePtr sequence(getSequence());
    for ( AVCodecID id : model::render::VideoCodecs::all() )
    {
        std::ostringstream os; os << "Render " << id;
        StartTest(os.str().c_str());
        WindowTriggerMenu(ID_RENDERSETTINGS);
        gui::DialogRenderSettings::get().getVideoCodecButton()->select(id);
        ButtonTriggerPressed(gui::DialogRenderSettings::get().getOkButton());
        RenderAndPlaybackCurrentTimeline();
        ProjectViewOpenTimelineForSequence(sequence);
    }
}

void TestRender::testRenderingTransition()
{
    StartTestSuite();

    TrimLeft(VideoClip(0),60);
    MakeInOutTransitionAfterClip preparation(0);
    preparation.dontUndo();

    RenderAndPlaybackCurrentTimeline(2, 20, 1000);
}

void TestRender::testRenderingEmptyClip()
{
    StartTestSuite();

    TimelineLeftClick(Center(VideoClip(0,0)));
    TimelineKeyPress(WXK_DELETE);

    RenderAndPlaybackCurrentTimeline();
}

void TestRender::testRenderingTransformedClip()
{
    StartTestSuite();

    ConfigFixture.SnapToClips(true);
    WindowTriggerMenu(ID_ADDAUDIOTRACK);
    WindowTriggerMenu(ID_ADDVIDEOTRACK);
    ConfigOverruleLong overrule(Config::sPathDebugMaxRenderLength, 3); // Only render 3s
    wxFileName path(wxFileName::GetTempDir(), "out", "avi");
    TimelineDragToTrack(1, VideoClip(0,1), AudioClip(0,1));
    TimelineDrag(From(Center(VideoClip(1,1))).To(LeftCenter(VideoClip(1,0)) + wxPoint(5,0)));
    model::VideoClipPtr clip = boost::dynamic_pointer_cast<model::VideoClip>(VideoClip(1,0));
    ASSERT(clip);
    RunInMainAndWait([clip]{ clip->setOpacity(128); });
    RunInMainAndWait([clip]{ clip->setScaling(model::VideoScalingCustom, boost::optional<boost::rational< int > >(1,2)); });
    RunInMainAndWait([clip]{ clip->setPosition(wxPoint(50,50)); });

    RenderAndPlaybackCurrentTimeline();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

std::pair< RandomTempDirPtr, wxFileName > TestRender::OpenDialogAndSetFilename()
{
    WindowTriggerMenu(ID_RENDERSETTINGS);
    
    RandomTempDirPtr tempdir = boost::make_shared<RandomTempDir>();
    wxFileName path(tempdir->getFileName().GetFullPath(), "out", "avi");
    gui::Dialog::get().setSaveFile(path.GetFullPath());
    ButtonTriggerPressed(gui::DialogRenderSettings::get().getFileButton());
    return std::make_pair(tempdir,path);
}

std::pair< RandomTempDirPtr, wxFileName > TestRender::RenderTimeline(int lengthInS)
{
    ConfigOverruleLong overrule(Config::sPathDebugMaxRenderLength, lengthInS);
    std::pair< RandomTempDirPtr, wxFileName > tempdir_and_filename = OpenDialogAndSetFilename();
    ExpectExecutedWork expectation(1);
    ButtonTriggerPressed(gui::DialogRenderSettings::get().getRenderButton());
    expectation.wait();
    ASSERT(tempdir_and_filename.second.Exists());
    return tempdir_and_filename;
}

void TestRender::PlaybackRenderedTimeline(const wxFileName& path, pixel start, milliseconds t)
{
    model::FolderPtr folder1 = ProjectViewAddFolder( "PlaybackRenderedTimeline" );
    model::Files files1 = ProjectViewAddFiles( boost::assign::list_of(path), folder1 );
    model::SequencePtr sequence1 = ProjectViewCreateSequence( folder1 );
    Zoom level(4);
    Play(start, t);
    ProjectViewRemove(sequence1);
    ProjectViewRemove(folder1);
}

void TestRender::RenderAndPlaybackCurrentTimeline(int renderedlengthInS, pixel playbackStart, milliseconds playbackLength)
{
    RandomTempDir tempdir;
    std::pair< RandomTempDirPtr, wxFileName > tempdir_and_filename =  RenderTimeline(renderedlengthInS);
    WindowTriggerMenu(ID_CLOSESEQUENCE);
    PlaybackRenderedTimeline(tempdir_and_filename.second, playbackStart, playbackLength);
}

} // namespace