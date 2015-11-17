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

#include "TestFileTypesVideo.h"

namespace test {

void executeVideoTest(wxString filetypesDir)
{
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

void TestFileTypesVideo::testFileTypes_video_s24p()
{
    StartTestSuite();
    ConfigOverrule<wxString> overruleFrameRate(Config::sPathVideoDefaultFrameRate, FrameRate::s24p.toString());
    executeVideoTest("filetypes_video");
}

void TestFileTypesVideo::testFileTypes_video_s25p()
{
    StartTestSuite();
    ConfigOverrule<wxString> overruleFrameRate(Config::sPathVideoDefaultFrameRate, FrameRate::s25p.toString());
    executeVideoTest("filetypes_video");
}

void TestFileTypesVideo::testFileTypes_video_s29p()
{
    StartTestSuite();
    ConfigOverrule<wxString> overruleFrameRate(Config::sPathVideoDefaultFrameRate, FrameRate::s30p.toString());
    executeVideoTest("filetypes_video");
}

} // namespace
