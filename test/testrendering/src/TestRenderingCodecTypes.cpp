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

#include "TestRenderingCodecTypes.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestRenderingCodecTypes::setUp()
{
    mProjectFixture.init();
}

void TestRenderingCodecTypes::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestRenderingCodecTypes::testRenderingCodecsVideo()
{
    StartTestSuite();
    model::SequencePtr sequence(getSequence());
    for ( AVCodecID id : model::render::VideoCodecs::all() )
    {
        std::ostringstream os; os << "Render " << id;
        StartTest(os.str().c_str());
        WindowTriggerMenu(ID_RENDERSETTINGS);
        WaitUntilMainWindowActive(false);
        gui::DialogRenderSettings::get().getVideoCodecButton()->select(id);
        ButtonTriggerPressed(gui::DialogRenderSettings::get().getOkButton());
        RenderAndPlaybackCurrentTimeline();
        ProjectViewOpenTimelineForSequence(sequence);
    }
}


void TestRenderingCodecTypes::testRenderingCodecsAudio()
{
    StartTestSuite();
    model::SequencePtr sequence(getSequence());
    for (int nChannels = 1; nChannels <= 2; ++nChannels)
    {
        util::thread::RunInMainAndWait([] {model::Properties::get().setAudioNumberOfChannels(1); });
        for (AVCodecID id : model::render::AudioCodecs::all())
        {
            std::ostringstream os; os << "Render " << id << (nChannels == 1 ? " mono" : " stereo");
            StartTest(os.str().c_str());
            WindowTriggerMenu(ID_RENDERSETTINGS);
            WaitUntilMainWindowActive(false);
            gui::DialogRenderSettings::get().getAudioCodecButton()->select(id);
            ButtonTriggerPressed(gui::DialogRenderSettings::get().getOkButton());
            RenderAndPlaybackCurrentTimeline();
            ProjectViewOpenTimelineForSequence(sequence);
        }
    }
}

} // namespace