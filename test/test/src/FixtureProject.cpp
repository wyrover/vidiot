#include "FixtureProject.h"

#include "AutoFolder.h"
#include "HelperProjectView.h"
#include "HelperTimeline.h"
#include "HelperTimelinesView.h"
#include "IClip.h"
#include "Project.h"
#include "Sequence.h"
#include "SuiteCreator.h"
#include "Track.h"
#include "UtilLog.h"
#include <boost/foreach.hpp>
#include <wx/utils.h>

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

FixtureProject::FixtureProject()
{
}

FixtureProject::~FixtureProject()
{
}

void FixtureProject::init()
{
    if (ISuite::currentTestIsDisabled()) { return; } // Test was disabled

    wxString sVidiotDir;
    bool found = wxGetEnv( _T("VIDIOT_DIR"), &sVidiotDir);
    ASSERT(found);
    TestFilesPath = wxFileName(sVidiotDir + "\\test", "");

    mRoot = createProject();
    ASSERT(mRoot);
    mAutoFolder = addAutoFolder( TestFilesPath );
    ASSERT_EQUALS(mAutoFolder->getParent(),mRoot);
    mSequence = createSequence( mAutoFolder );

    ASSERT_EQUALS(mSequence->getParent(),mRoot);

    InputFiles = model::AutoFolder::getSupportedFiles(TestFilesPath);

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
    // Click in the timeline to give it the focus. A lot of test cases start
    // with zooming in via keyboard commands. For that purpose, timeline must
    // have the current focus.
    Click(wxPoint(2,2));

}

void FixtureProject::destroy()
{
    if (ISuite::currentTestIsDisabled()) { return; } // Test was disabled

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