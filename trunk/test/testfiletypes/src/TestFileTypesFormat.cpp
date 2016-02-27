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

void executeFormatTest(wxString filetypesDir, bool audio, bool video)
{
    ExecuteOnAllFiles(filetypesDir, [audio, video] 
    {
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
        model::IClipPtr clip{ VideoClip(0,0) };
        Play(LeftPixel(clip), 1000);
        TimelinePositionCursor(HCenter(clip));
        TimelineKeyPress('v'); // Show the part being played (for longer files)
        Play(HCenter(clip), 1000);
    }, true);
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestFileTypesFormat::testFileTypes_formats_audio()
{
    StartTestSuite();
    executeFormatTest("filetypes_formats_audio", true, false);
}

void TestFileTypesFormat::testFileTypes_formats_audio_and_video()
{
    StartTestSuite();
    executeFormatTest("filetypes_formats_audio_and_video", true, true);
}

void TestFileTypesFormat::testFileTypes_formats_video()
{
    StartTestSuite();
    executeFormatTest("filetypes_formats_video", false, true);
}


} // namespace
