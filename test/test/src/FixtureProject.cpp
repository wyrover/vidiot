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

#include "Test.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

FixtureProject::FixtureProject(bool focusTimeline)
    : mFocusTimeline(focusTimeline)
{
}

FixtureProject::~FixtureProject()
{
}

void FixtureProject::init()
{
    if (!HelperTestSuite::get().currentTestIsEnabled()) { return; } // Test was disabled

    mRoot = WindowCreateProject();
    ASSERT(mRoot);
    InputFiles = getListOfInputPaths();

    mAutoFolder = ProjectViewAddAutoFolder( getTestFilesPath() );
    WaitForChildCount(mRoot, InputFiles.size() + 2); // +2: Root + autofolder
    ASSERT_EQUALS(mAutoFolder->getParent(),mRoot);
    ASSERT_EQUALS(mAutoFolder->getChildren().size(), InputFiles.size());
    mSequence = ProjectViewCreateSequence( mAutoFolder );

    ASSERT_EQUALS(mSequence->getParent(),mRoot);

    for ( model::IClipPtr clip : mSequence->getVideoTrack(0)->getClips() )
    {
        mOriginalLengthOfVideoClip.push_back(clip->getLength());
        mOriginalPtsOfVideoClip.push_back(clip->getLeftPts());
    }
    for ( model::IClipPtr clip : mSequence->getAudioTrack(0)->getClips() )
    {
        mOriginalLengthOfAudioClip.push_back(clip->getLength());
        mOriginalPtsOfAudioClip.push_back(clip->getLeftPts());
    }

    WaitForIdle;
    if (mFocusTimeline && FixtureGui::UseRealUiEvents)
    {
        // Click in the timeline to give it the focus. A lot of test cases start
        // with zooming in via keyboard commands. For that purpose, timeline must
        // have the current focus.
        MouseMoveWithinWidget(wxPoint(2,2),TimelinePosition()); // No asserting on timelineposition here since timeline may not have focus yet
        TimelineLeftDown();
        TimelineLeftUp();
    }
}

void FixtureProject::destroy()
{
    if (!HelperTestSuite::get().currentTestIsEnabled()) { return; } // Test was disabled

    // Must be done here, since the deletion of files causes logging.
    // Logging is stopped (unavailable) after tearDown since application window is closed.
    InputFiles.clear();

    mSequence.reset();
    mAutoFolder.reset();
    mRoot.reset();
}

pts FixtureProject::OriginalLengthOfVideoClip(int trackindex, int clipindex)
{
    ASSERT_ZERO(trackindex); // Other tracks are not stored
    ASSERT_LESS_THAN(static_cast<size_t>(clipindex),mOriginalLengthOfVideoClip.size());
    return mOriginalLengthOfVideoClip[clipindex];
}

pts FixtureProject::OriginalPtsOfVideoClip(int trackindex, int clipindex)
{
    ASSERT_ZERO(trackindex); // Other tracks are not stored
    ASSERT_LESS_THAN(static_cast<size_t>(clipindex),mOriginalPtsOfVideoClip.size());
    return mOriginalPtsOfVideoClip[clipindex];
}

pts FixtureProject::OriginalLengthOfAudioClip(int trackindex, int clipindex)
{
    ASSERT_ZERO(trackindex); // Other tracks are not stored
    ASSERT_LESS_THAN(static_cast<size_t>(clipindex),mOriginalLengthOfAudioClip.size());
    return mOriginalLengthOfAudioClip[clipindex];
}

pts FixtureProject::OriginalPtsOfAudioClip(int trackindex, int clipindex)
{
    ASSERT_ZERO(trackindex); // Other tracks are not stored
    ASSERT_LESS_THAN(static_cast<size_t>(clipindex),mOriginalPtsOfAudioClip.size());
    return mOriginalPtsOfAudioClip[clipindex];
}

} // namespace
