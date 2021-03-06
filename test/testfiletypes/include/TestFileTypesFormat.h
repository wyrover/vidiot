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

#pragma once

#include "Test.h"

namespace test {

class TestFileTypesFormat: public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestFileTypesFormat>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // TEST CASES
    //////////////////////////////////////////////////////////////////////////

    void testFileTypes_formats_new();

    /// For checking that certain formats can be opened.
    void testFileTypes_formats_audio(); 
    void testFileTypes_formats_audio_and_video();
    void testFileTypes_formats_video();

    /// The file used in this test is read with a wrong duration.
    /// Therefore, playback is not done at the default positions.
    void testFileTypes_formats_audio_with_wrong_duration();

    /// User reported error. A file format was used in which both an audio stream and
    /// a video stream were present. The audio stream, however, did not contain any
    /// actual data. Then, after trimming the clip, generating audio peaks caused
    /// an assert to fail.
    void testFileTypes_formats_empty_audio_channel();

    /// See testFileTypes_formats_empty_audio_channel. Same test, but now with empty
    /// video channel. Not an actual bug, but may become one ;-)
    void testFileTypes_formats_empty_video_channel();

};

} // namespace

using namespace test;
