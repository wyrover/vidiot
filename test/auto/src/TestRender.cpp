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

#include "Dialog.h"
#include "DialogRenderSettings.h"
#include "Render.h"
#include "UtilLogAvcodec.h" // For logging codec ids. Still compiles without this include but then the title bar shows ints, not text for the codec name.
#include "VideoCodecs.h"
#include "Worker.h"

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
        model::render::RenderPtr original = getCurrentRenderSettings();
        triggerMenu(ID_RENDERSETTINGS);
        gui::Dialog::get().setSaveFile("D:/out.avi");
        ClickTopLeft(gui::DialogRenderSettings::get().getFileButton());
        ClickBottomLeft(gui::DialogRenderSettings::get().getVideoParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickBottomLeft(gui::DialogRenderSettings::get().getVideoParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickBottomLeft(gui::DialogRenderSettings::get().getVideoParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickBottomLeft(gui::DialogRenderSettings::get().getVideoParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickTopLeft(gui::DialogRenderSettings::get().getCancelButton());
        model::render::RenderPtr current = getCurrentRenderSettings();
        ASSERT_EQUALS(*original,*current);
    }
    {
        StartTest("If apply is pressed, the sequence is changed (with a different video codec setting).");
        triggerMenu(ID_RENDERSETTINGS);
        gui::Dialog::get().setSaveFile("D:/out.avi");
        ClickTopLeft(gui::DialogRenderSettings::get().getFileButton());
        model::render::RenderPtr original = getCurrentRenderSettings();
        ClickBottomLeft(gui::DialogRenderSettings::get().getVideoParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickBottomLeft(gui::DialogRenderSettings::get().getVideoParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickBottomLeft(gui::DialogRenderSettings::get().getVideoParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickTopLeft(gui::DialogRenderSettings::get().getApplyButton());
        model::render::RenderPtr current = getCurrentRenderSettings();
        ASSERT_DIFFERS(*original,*current);
        ClickTopLeft(gui::DialogRenderSettings::get().getCancelButton());
    }
    {
        StartTest("If apply is pressed, the sequence is changed (with a different audio codec setting).");
        triggerMenu(ID_RENDERSETTINGS);
        gui::Dialog::get().setSaveFile("D:/out.avi");
        ClickTopLeft(gui::DialogRenderSettings::get().getFileButton());
        model::render::RenderPtr original = getCurrentRenderSettings();
        ClickBottomLeft(gui::DialogRenderSettings::get().getAudioParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickBottomLeft(gui::DialogRenderSettings::get().getAudioParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickBottomLeft(gui::DialogRenderSettings::get().getAudioParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickBottomLeft(gui::DialogRenderSettings::get().getAudioParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickTopLeft(gui::DialogRenderSettings::get().getApplyButton());
        model::render::RenderPtr current = getCurrentRenderSettings();
        ASSERT_DIFFERS(*original,*current);
        ClickTopLeft(gui::DialogRenderSettings::get().getCancelButton());
    }
    {
        StartTest("If OK is pressed, the sequence is changed and the dialog is closed.");
        triggerMenu(ID_RENDERSETTINGS);
        gui::Dialog::get().setSaveFile("D:/out.avi");
        ClickTopLeft(gui::DialogRenderSettings::get().getFileButton());
        model::render::RenderPtr original = getCurrentRenderSettings();
        ClickBottomLeft(gui::DialogRenderSettings::get().getVideoParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickBottomLeft(gui::DialogRenderSettings::get().getVideoParam(0),wxPoint(4,-4));  // Click on the down symbol. Note that the position returned by getscreenposition is the top left pixel of the spin button. The text field is 'ignored'.
        ClickTopLeft(gui::DialogRenderSettings::get().getOkButton());
        model::render::RenderPtr current = getCurrentRenderSettings();
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
        Click(Center(VideoClip(0,2)));
        ControlDown();
        Click(Center(VideoClip(0,4))); // Exclude clip 3 deliberately: include empty clips in the list of intervals
        Click(Center(VideoClip(0,5)));
        Click(Center(VideoClip(0,6)));
        ControlUp();
        Type(WXK_DELETE);
        RandomTempDir tempdir;
        model::render::RenderPtr original = getCurrentRenderSettings();
        triggerMenu(ID_RENDERSETTINGS);
        wxFileName fn(tempdir.getFileName().GetLongPath(), "out" ,"avi");
        gui::Dialog::get().setSaveFile(fn.GetLongPath());
        ClickTopLeft(gui::DialogRenderSettings::get().getFileButton());
        ClickTopLeft(gui::DialogRenderSettings::get().getRenderSeparationCheckBox(),wxPoint(4,4));
        ClickTopLeft(gui::DialogRenderSettings::get().getRenderButton());
        expectation.wait();
        for (int i = 1; i <= 4; ++i)
        {
            wxFileName f(tempdir.getFileName().GetLongPath(), wxString::Format("out_%d",i), "avi");
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
        std::ostringstream osCodec; osCodec << id;
        std::ostringstream os; os << "Render " << osCodec.str();// << " into " << path.GetLongPath();
        StartTest(os.str().c_str());
        triggerMenu(ID_RENDERSETTINGS);
        gui::DialogRenderSettings::get().getVideoCodecButton()->select(id);
        ClickTopLeft(gui::DialogRenderSettings::get().getOkButton());
        RenderAndPlaybackCurrentTimeline();
        openTimelineForSequence(sequence);
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

    Click(Center(VideoClip(0,0)));
    Type(WXK_DELETE);

    RenderAndPlaybackCurrentTimeline();
}

void TestRender::testRenderingTransformedClip()
{
    StartTestSuite();

    ConfigFixture.SnapToClips(true);
    triggerMenu(ID_ADDAUDIOTRACK);
    triggerMenu(ID_ADDVIDEOTRACK);
    ConfigOverruleLong overrule(Config::sPathDebugMaxRenderLength, 3); // Only render 3s
    wxFileName path(wxFileName::GetTempDir(), "out", "avi");
    DragToTrack(1, VideoClip(0,1), AudioClip(0,1));
    Drag(From(Center(VideoClip(1,1))).To(LeftCenter(VideoClip(1,0)) + wxPoint(5,0)));
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

void TestRender::RenderTimelineInto(const wxFileName& path, int lengthInS)
{
    ConfigOverruleLong overrule(Config::sPathDebugMaxRenderLength, lengthInS);
    triggerMenu(ID_RENDERSETTINGS);
    gui::Dialog::get().setSaveFile(path.GetFullPath());
    ClickTopLeft(gui::DialogRenderSettings::get().getFileButton());
    waitForIdle();
    ExpectExecutedWork expectation(1);
    ClickTopLeft(gui::DialogRenderSettings::get().getRenderButton());
    expectation.wait();
    ASSERT(path.Exists());
}

void TestRender::PlaybackRenderedTimeline(const wxFileName& path, pixel start, milliseconds t)
{
    model::FolderPtr folder1 = addFolder( "PlaybackRenderedTimeline" );
    model::Files files1 = addFiles( boost::assign::list_of(path), folder1 );
    model::SequencePtr sequence1 = createSequence( folder1 );
    Zoom level(4);
    Play(start, t);
    remove(sequence1);
    remove(folder1);
}

void TestRender::RenderAndPlaybackCurrentTimeline(int renderedlengthInS, pixel playbackStart, milliseconds playbackLength)
{
    RandomTempDir tempdir;
    wxFileName path(tempdir.getFileName().GetFullPath(), "out", "avi");
    RenderTimelineInto(path, renderedlengthInS);
    triggerMenu(ID_CLOSESEQUENCE);
    PlaybackRenderedTimeline(path, playbackStart, playbackLength);
}

} // namespace