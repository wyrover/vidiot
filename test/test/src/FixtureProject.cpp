#include "FixtureProject.h"

#include <wx/utils.h>
#include "AutoFolder.h"
#include "HelperProjectView.h"
#include "HelperTimeline.h"
#include "Project.h"
#include "Sequence.h"

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

    // Click in the timeline to give it the focus. A lot of test cases start
    // with zooming in via keyboard commands. For that purpose, timeline must
    // have the current focus.
    Click(wxPoint(2,2));

}

void FixtureProject::destroy()
{
    // Must be done here, since the deletion of files causes logging.
    // Logging is stopped (unavailable) after tearDown since application window is closed.
    InputFiles.clear();

    mSequence.reset();
    mAutoFolder.reset();
    mRoot.reset();
}

} // namespace