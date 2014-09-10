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

#include "TestSyncFileTypes.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestSyncFileTypes::testSync_s24p_44100()
{
    StartTestSuite();
    ConfigOverruleString overruleFrameRate(Config::sPathDefaultFrameRate, FrameRate::s24p.toString());
    ConfigOverruleLong overruleChannels(Config::sPathDefaultAudioChannels, 1);
    ConfigOverruleLong overruleSampleRate(Config::sPathDefaultAudioSampleRate, 44100);
    executeTest();
}

void TestSyncFileTypes::testSync_s25p_48000()
{
    StartTestSuite();
    ConfigOverruleString overruleFrameRate(Config::sPathDefaultFrameRate, FrameRate::s25p.toString());
    ConfigOverruleLong overruleChannels(Config::sPathDefaultAudioChannels, 1);
    ConfigOverruleLong overruleSampleRate(Config::sPathDefaultAudioSampleRate, 44100);
    executeTest();
}

void TestSyncFileTypes::testSync_s30p_48000()
{
    StartTestSuite();
    ConfigOverruleString overruleFrameRate(Config::sPathDefaultFrameRate, FrameRate::s30p.toString());
    ConfigOverruleLong overruleChannels(Config::sPathDefaultAudioChannels, 2);
    ConfigOverruleLong overruleSampleRate(Config::sPathDefaultAudioSampleRate, 44100);
    executeTest();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void TestSyncFileTypes::executeTest()
{
    // Input file from:
    // https://www.youtube.com/watch?v=cGgf_dbDMsw - Audio Video Sync Test & Calibration 23.976fps CelestialDLS
    // https://www.youtube.com/watch?v=EoeDZkGvvjg&list=UUPbXqPG7SS98ogtEScFRyjQ - Audio Video Sync Test & Calibration 29.97fps CelestialDLS

    ExecuteOnAllFiles("filetypes_sync", [] 
    {
        Play(LeftPixel(VideoClip(0,0)), 20000);
        Play(HCenter(VideoClip(0,0)), 5000);
    });

}

} // namespace