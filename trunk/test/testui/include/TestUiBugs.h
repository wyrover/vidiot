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

#pragma once

#include "Test.h"

namespace test {

class TestUiBugs : public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestUiBugs>
{
public:
    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    virtual void setUp();       ///< Called before each test.
    virtual void tearDown();    ///< Called after each test.

    //////////////////////////////////////////////////////////////////////////
    // TEST CASES
    //////////////////////////////////////////////////////////////////////////

    void testDragAndDropVideoFileLargerThanTimeline();

    void testDragAndDropAudioFileLargerThanTimeline();

    /// Wrong handling in menu class - did not check for 'no clip selected' -
    /// caused a crash when accessing the clip's data.
    void testCrashWhenRightClickingOutsideAllTracks();

    /// This caused flickering feedback during the drag operation AND a crash
    /// after deleting the dropped clip (the dropped clip's link was nonexistent).
    void testDragAndDropWithAddingTracksWithSelectedClipInTopMostTrackForVideo();

    /// This caused flickering feedback during the drag operation AND a crash
    /// after deleting the dropped clip (the dropped clip's link was nonexistent).
    void testDragAndDropWithAddingTracksWithSelectedClipInTopMostTrackForAudio();

    /// \see testDragAndDropWithAddingTracksWithSelectedClipInTopMostTrackForVideo
    /// Same scenario but with a media file that ONLY contained video.
    /// This caused a crash during the drag operation.
    void testDragAndDropWithAddingTracksWithSelectedClipInTopMostTrackForVideoOnly();

    /// \see testDragAndDropWithAddingTracksWithSelectedClipInTopMostTrackForAudio
    /// Same scenario but with a media file that ONLY contained audio.
    /// This caused a crash during the drag operation.
    void testDragAndDropWithAddingTracksWithSelectedClipInTopMostTrackForAudioOnly();

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    FixtureProject mProjectFixture;

};

} // namespace

using namespace test;
