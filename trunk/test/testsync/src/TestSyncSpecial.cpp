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

#include "TestSyncSpecial.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestSyncSpecial::test_AudioAndVideoStreamsHaveDifferentStartTimes()
{
    StartTestSuite();
    ConfigOverrule<long> overruleChannels(Config::sPathAudioDefaultNumberOfChannels, 2);
    ExecuteOnAllFiles("filetypes_formats_audio_and_video/ffmp2-dump_not_all_streams_have_the_same_start_time.ts", []
    {
        model::SequencePtr sequence{ getSequence() };

        model::AudioCompositionParameters audioParameters =
            model::AudioCompositionParameters().setNrChannels(2).setSampleRate(48000).setPts(0);
        model::VideoCompositionParameters videoParameters =
            model::VideoCompositionParameters().setBoundingBox(wxSize{ 1280,1024 }).setOptimizeForQuality().setPts(0);

        TimelinePositionCursor(0);

        ASSERT_AUDIO_CHUNK(sequence->getNextAudio(audioParameters), { -147,159,-178,144,-251,96,-388,-26,-540,-190,-687 });
#ifdef _MSC_VER
        ASSERT_VIDEO_FRAME(sequence->getNextVideo(videoParameters), { 106, 100, 100, 106, 102, 79, 62 }, { 52, 51, 53, 65, 68, 51, 38 }, { 55, 52, 57, 68, 68, 49, 35 });
#else
        ASSERT_VIDEO_FRAME(sequence->getNextVideo(videoParameters), { 112,105,95,91,94,101,89 }, { 56,49,49,46,55,62,63 }, { 62,55,54,51,59,66,61 });
#endif

        TimelinePositionCursor(getTimeline().getZoom().timeToPixels(4000));

        ASSERT_AUDIO_CHUNK(sequence->getNextAudio(audioParameters), { 579,-2280,270,-2439,-225,-2484,-778,-2497,-1225,-2566,-1456 });
        ASSERT_VIDEO_FRAME(sequence->getNextVideo(videoParameters), { 81,81,80,77,76,76,80 }, { 55,55,54,52,51,51,54 }, { 51,51,52,49,48,48,49 });

        Play("0:00", 3000);
        Play("0:04", 3000);
    }, true);
}

} // namespace
