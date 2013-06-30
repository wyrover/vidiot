#include "FixtureProject.h"

#include "AutoFolder.h"
#include "HelperApplication.h"
#include "HelperFileSystem.h"
#include "HelperProjectView.h"
#include "HelperTestSuite.h"
#include "HelperTimeline.h"
#include "IClip.h"
#include "Sequence.h"
#include "Track.h"
#include "UtilLog.h"

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

#ifndef SOURCE_ROOT
#error "SOURCE_ROOT is not defined!"
#endif

    mRoot = createProject();
    ASSERT(mRoot);
    InputFiles = getListOfInputFiles();

    mAutoFolder = addAutoFolder( getTestFilesPath() );
    WaitForChildCount(mRoot, InputFiles.size() + 2); // +2: Root + autofolder
    ASSERT_EQUALS(mAutoFolder->getParent(),mRoot);
    ASSERT_EQUALS(mAutoFolder->getChildren().size(), InputFiles.size());
    mSequence = createSequence( mAutoFolder );

    ASSERT_EQUALS(mSequence->getParent(),mRoot);

    BOOST_FOREACH( model::IClipPtr clip, mSequence->getVideoTrack(0)->getClips() )
    {
        mOriginalLengthOfVideoClip.push_back(clip->getLength());
        mOriginalPtsOfVideoClip.push_back(clip->getLeftPts());
    }
    BOOST_FOREACH( model::IClipPtr clip, mSequence->getAudioTrack(0)->getClips() )
    {
        mOriginalLengthOfAudioClip.push_back(clip->getLength());
        mOriginalPtsOfAudioClip.push_back(clip->getLeftPts());
    }

    waitForIdle();
    if (mFocusTimeline)
    {

        // Click in the timeline to give it the focus. A lot of test cases start
        // with zooming in via keyboard commands. For that purpose, timeline must
        // have the current focus.
        Click(wxPoint(2,2));
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