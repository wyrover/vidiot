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

#include "TestFileTypes.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestFileTypes::setUp()
{
#ifndef SOURCE_ROOT
#error "SOURCE_ROOT is not defined!"
#endif
}

void TestFileTypes::tearDown()
{
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestFileTypes::testFileTypes_audio_1_44100()
{
    StartTestSuite();
    ConfigOverruleLong overruleChannels(Config::sPathDefaultAudioChannels, 1);
    ConfigOverruleLong overruleSampleRate(Config::sPathDefaultAudioSampleRate, 44100);
    executeTest("filetypes_audio");
}

void TestFileTypes::testFileTypes_audio_2_48000()
{
    StartTestSuite();
    ConfigOverruleLong overruleChannels(Config::sPathDefaultAudioChannels, 2);
    ConfigOverruleLong overruleSampleRate(Config::sPathDefaultAudioSampleRate, 48000);
    executeTest("filetypes_audio");
}

void TestFileTypes::testFileTypes_image()
{
    StartTestSuite();
    executeTest("filetypes_image");
}

void TestFileTypes::testFileTypes_video_s24p()
{
    StartTestSuite();
    ConfigOverruleString overruleFrameRate(Config::sPathDefaultFrameRate, FrameRate::s24p.toString());
    executeTest("filetypes_video");
}

void TestFileTypes::testFileTypes_video_s25p()
{
    StartTestSuite();
    ConfigOverruleString overruleFrameRate(Config::sPathDefaultFrameRate, FrameRate::s25p.toString());
    executeTest("filetypes_video");
}

void TestFileTypes::testFileTypes_video_s29p()
{
    StartTestSuite();
    ConfigOverruleString overruleFrameRate(Config::sPathDefaultFrameRate, FrameRate::s30p.toString());
    executeTest("filetypes_video");
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void TestFileTypes::executeTest(wxString filetypesDir)
{
    // Wav files from: http://www-mmsp.ece.mcgill.ca/documents/AudioFormats/WAVE/Samples.html
    // Some files from http://samples.mplayerhq.hu/

    ExecuteOnAllFiles(filetypesDir, [] 
    {
        Play(LeftPixel(VideoClip(0,0)), 1000);
        Play(HCenter(VideoClip(0,0)), 1000);

    });
}

} // namespace
