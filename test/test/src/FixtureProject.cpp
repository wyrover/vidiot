#include "FixtureProject.h"

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
    TestFilesPath = wxFileName("D:\\Vidiot\\test", "");
    model::FolderPtr root = createProject();
    model::FolderPtr autofolder1 = addAutoFolder( TestFilesPath );
    model::SequencePtr sequence1 = createSequence( autofolder1 );
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
}

} // namespace