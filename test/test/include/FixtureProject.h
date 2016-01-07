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

class FixtureProject
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    FixtureProject(bool focusTimeline = true);
    virtual ~FixtureProject();

    /// Create objects here. During construction the application fixture is
    /// not yet fully created. This method must be called in a test's setup
    /// method.
    void init();

    /// Destruct objects here. This method must be called in a test's
    /// teardown method.
    void destroy();

    /// Some problems only occur after loading a saved document.
    /// After creating the proper structures (in the test), this method
    /// ensures that the project is saved to disk and then reread.
    /// \note store the result in a local variable to guarantee the 
    ///       lifetime of the project on disk.
    DirAndFile saveAndReload();

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    bool mFocusTimeline;                    ///< True if the timeline must be focused
    model::IPaths InputFiles;               ///< Contains the filenames of the input files in the test directory

    model::FolderPtr mRoot;
    model::FolderPtr mAutoFolder;
    model::SequencePtr mSequence;

    pts OriginalLengthOfVideoClip(int trackindex = 0, int clipindex = 0);
    pts OriginalPtsOfVideoClip(int trackindex = 0, int clipindex = 0);
    pts OriginalLengthOfAudioClip(int trackindex = 0, int clipindex = 0);
    pts OriginalPtsOfAudioClip(int trackindex = 0, int clipindex = 0);

    std::vector<pts> mOriginalLengthOfVideoClip;
    std::vector<pts> mOriginalPtsOfVideoClip;
    std::vector<pts> mOriginalLengthOfAudioClip;
    std::vector<pts> mOriginalPtsOfAudioClip;
};

} // namespace
