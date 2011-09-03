#include "TestProjectView.h"

#include <boost/assign/list_of.hpp>
#include "AutoFolder.h"
#include "File.h"
#include "FixtureApplication.h"
#include "HelperWindow.h"
#include "HelperProjectView.h"
#include "Sequence.h"
#include "UtilLog.h"

namespace test {


//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void ProjectViewTests::setUp()
{
    mProjectFixture.init();
}

void ProjectViewTests::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void ProjectViewTests::testAdditionAndRemoval()
{
    LOG_DEBUG << "TEST_START";

    int nDefaultItems = countProjectView();

    wxString sFolder1( "Folder1" );
    wxString sSequence1( "Sequence1" );
    wxString sFile( "scene'20100102 12.32.48.avi" ); // Should be a file also in the autofolder
    wxFileName filepath = wxFileName( "D:\\Vidiot\\test", sFile );

    model::FolderPtr folder1 = addFolder( sFolder1 );
    model::SequencePtr sequence1 = addSequence( sSequence1, folder1 );
    model::Files files1 = addFiles( boost::assign::list_of(filepath), folder1 );

    ASSERT_EQUALS(folder1->getParent(),mProjectFixture.mRoot);
    ASSERT_EQUALS(sequence1->getParent(),folder1);
    ASSERT_EQUALS(countProjectView(), nDefaultItems + 3); // Added: Folder + Sequence + File
    ASSERT_EQUALS(files1.size(),1);
    ASSERT_EQUALS(files1.front()->getParent(),folder1);
    ASSERT_EQUALS(mProjectFixture.mRoot->find(sFile).size(),1);    // One file with a relative file name
    ASSERT_EQUALS(mProjectFixture.mRoot->find(filepath.GetLongPath()).size(),1); // And one file with an absolute file name

    remove( files1.front() );
    ASSERT_EQUALS(countProjectView(), nDefaultItems + 2); // Added: Folder + Sequence
    remove( folder1 ); // Also removes sequence1 which is contained in folder1
    ASSERT_EQUALS(countProjectView(), nDefaultItems); // Added: None
}

} // namespace