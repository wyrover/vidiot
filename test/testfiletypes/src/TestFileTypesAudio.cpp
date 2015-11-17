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

#include "TestFileTypesAudio.h"

namespace test {

void executeAudioTest(wxString filetypesDir)
{
    // Wav files from: http://www-mmsp.ece.mcgill.ca/documents/AudioFormats/WAVE/Samples.html
    // Some files from http://samples.mplayerhq.hu/

    ExecuteOnAllFiles(filetypesDir, [] 
    {
        Play(LeftPixel(VideoClip(0,0)), 1000);
        TimelinePositionCursor(HCenter(AudioClip(0,0)));
        TimelineKeyPress('v'); // Show the part being played (for longer files)
        Play(HCenter(AudioClip(0,0)), 1000);
    }, true);
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestFileTypesAudio::testFileTypes_audio_1_44100()
{
    StartTestSuite();
    ConfigOverrule<long> overruleChannels(Config::sPathAudioDefaultNumberOfChannels, 1);
    ConfigOverrule<long> overruleSampleRate(Config::sPathAudioDefaultSampleRate, 44100);
    executeAudioTest("filetypes_audio");
}

void TestFileTypesAudio::testFileTypes_audio_2_48000()
{
    StartTestSuite();
    ConfigOverrule<long> overruleChannels(Config::sPathAudioDefaultNumberOfChannels, 2);
    ConfigOverrule<long> overruleSampleRate(Config::sPathAudioDefaultSampleRate, 48000);
    executeAudioTest("filetypes_audio");
}

} // namespace
