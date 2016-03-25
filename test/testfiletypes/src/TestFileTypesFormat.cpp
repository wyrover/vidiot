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

#include "TestFileTypesFormat.h"

namespace test {

void executeFormatTest(wxString path, bool audio, bool video)
{
    ExecuteOnAllFiles(path, [audio, video]
    {
        TimelineZoomIn(8);
        ASSERT(video || audio);


        // NOTE: For audio-only or video-only the counterpart becomes an EmptyClip
        if (!audio) 
        {
            ASSERT_AUDIOTRACK0(EmptyClip);
        }
        else
        {
            ASSERT_AUDIOTRACK0(AudioClip);
        }
        if (!video)
        {
            ASSERT_VIDEOTRACK0(EmptyClip);
        }
        else
        {
            ASSERT_VIDEOTRACK0(VideoClip);
        }
        // todo encode parameters of the file in the file name (length in #frames,  for example) such that these can be checked.

        model::IClipPtr clip{ VideoClip(0,0) };
        model::AudioCompositionParameters audioParameters{ model::AudioCompositionParameters().setPts(0) };
        model::VideoCompositionParameters videoParameters{ model::VideoCompositionParameters().setBoundingBox(wxSize{1280,1024}).setOptimizeForQuality().setPts(0) };

        ASSERT(!getSequence()->getNextAudio(audioParameters)->getError());
        ASSERT(!getSequence()->getNextVideo(videoParameters)->getError());
        Play(0, 1000);
        
        TimelinePositionCursor(HCenter(clip));
        TimelineKeyPress('v'); // Show the part being played (for longer files)
        audioParameters = model::AudioCompositionParameters(audioParameters).setPts(getTimeline().getZoom().pixelsToPts(HCenter(clip)));
        videoParameters = model::VideoCompositionParameters(videoParameters).setPts(getTimeline().getZoom().pixelsToPts(HCenter(clip)));
        ASSERT(!getSequence()->getNextAudio(audioParameters)->getError());
        ASSERT(!getSequence()->getNextVideo(videoParameters)->getError());
        Play(HCenter(clip), 1000);

        TimelinePositionCursor(RightPixel(clip) - 25);
        TimelineKeyPress('v'); // Show the part being played (for longer files)
        audioParameters = model::AudioCompositionParameters(audioParameters).setPts(getTimeline().getZoom().pixelsToPts(RightPixel(clip) - 25));
        videoParameters = model::VideoCompositionParameters(videoParameters).setPts(getTimeline().getZoom().pixelsToPts(RightPixel(clip) - 25));
        ASSERT(!getSequence()->getNextAudio(audioParameters)->getError());
        ASSERT(!getSequence()->getNextVideo(videoParameters)->getError());
        TimelinePositionCursor(RightPixel(clip) - 25);

        WaitForPlaybackStopped stopped;
        Play();
        stopped.wait();
        pause();
    }, true);
}
             //todo add option for converting single files immediately
//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestFileTypesFormat::testFileTypes_formats_new()
{
    StartTestSuite();
    ConfigOverrule<long> overruleChannels(Config::sPathAudioDefaultNumberOfChannels, 2);
    ConfigOverrule<long> overruleSampleRate(Config::sPathAudioDefaultSampleRate, 44100);
    //executeFormatTest(R"*(D:\samples\todo_movenotworking\frame_tags.yuv)*", false, true);
}

void TestFileTypesFormat::testFileTypes_formats_audio()
{
    StartTestSuite();
    ConfigOverrule<long> overruleChannels(Config::sPathAudioDefaultNumberOfChannels, 2);
    executeFormatTest("filetypes_formats_audio", true, false);
}

void TestFileTypesFormat::testFileTypes_formats_audio_and_video()
{
    StartTestSuite();
    ConfigOverrule<long> overruleChannels(Config::sPathAudioDefaultNumberOfChannels, 2);
    executeFormatTest("filetypes_formats_audio_and_video", true, true);
}

void TestFileTypesFormat::testFileTypes_formats_video()
{
    StartTestSuite();
    ConfigOverrule<long> overruleChannels(Config::sPathAudioDefaultNumberOfChannels, 2);
    executeFormatTest("filetypes_formats_video", false, true);
}

void TestFileTypesFormat::testFileTypes_formats_audio_with_wrong_duration()
{
    StartTestSuite();
    ConfigOverrule<long> overruleChannels(Config::sPathAudioDefaultNumberOfChannels, 2);
    ExecuteOnAllFiles("filetypes_special/2_aac_error_while_decoding_stream.aac", []
    {
        Play("0:04", 1000);
        Play("0:15", 1000);
        Play("1:35", 1000);
        Play("4:49", 2000);
    }, true);
}

} // namespace
