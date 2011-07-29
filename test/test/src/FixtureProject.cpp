#include "FixtureProject.h"

#include "AutoFolder.h"
#include "HelperProjectView.h"
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
    InputFiles = model::AutoFolder::getSupportedFiles(TestFilesPath);
    model::FolderPtr root = createProject();
    model::FolderPtr autofolder1 = addAutoFolder( TestFilesPath );
    model::SequencePtr sequence1 = createSequence( autofolder1 );
}

void FixtureProject::destroy()
{
    // Must be done here, since the deletion of files causes logging. 
    // Logging is stopped (unavailable) after tearDown since application window is closed.
    InputFiles.clear(); 
}

} // namespace