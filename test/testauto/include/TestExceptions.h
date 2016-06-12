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

class TestExceptions: public CxxTest::TestSuite // Must be on same line as class definition. Otherwise 'No tests defined error
    ,   public SuiteCreator<TestExceptions>
{
public:

    //////////////////////////////////////////////////////////////////////////
    // TEST CASES
    //////////////////////////////////////////////////////////////////////////

    /// Test handling of a file that is part of a sequence and then removed
    /// on disk, for video files.
    void testRemoveFileInSequence_Video();

    /// Test handling of a file that is part of a sequence and then removed
    /// on disk, for title files (handle by wxImageFile).
    void testRemovedFileInSequence_Title();

    /// Test opening a project with a timeline containing a video file, but
    /// the video file has been removed from disk.
    void testRemovedFileInSequenceBeforeOpening_Video();

    /// Test opening a project with a timeline containing a video file, but
    /// the video file has been removed from disk.
    void testRemovedFileInSequenceBeforeOpening_Title();

    void testRemovedFileUsedForTransitionsBeforeOpening();

    /// Particularly required for testing the generation of audio peaks for
    /// a missing file, where the speed triggers SoundTouch.
    void testRemovedFileWithAdjustedSpeed();

    void testRemovedFileInProjectViewBeforeOpening();

    void testRemovedFolderInProjectViewBeforeOpening();

private:

    //////////////////////////////////////////////////////////////////////////
    // HELPER METHODS
    //////////////////////////////////////////////////////////////////////////

    std::pair< model::SequencePtr, RandomTempDirPtr> createProjectWithOneFile(const wxFileName& file);

    void testRemovedFileInSequence(const wxFileName& file, std::function<void()> adjustment = std::function<void()>());

    void testRemovedFileInSequenceBeforeOpening(const wxFileName& file);

    wxFileName getFileName(wxString folder, wxString file) const;

};

} // namespace

using namespace test;
