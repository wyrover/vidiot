#include "TestProjectView.h"

#include "File.h"
#include "Folder.h"
#include "HelperProjectView.h"
#include "Sequence.h"

namespace test {

    //////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestProjectView::setUp()
{
    mProjectFixture.init();
}

void TestProjectView::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestProjectView::testAdditionAndRemoval()
{
    StartTestSuite();

    int nDefaultItems = countProjectView();

    wxString sFolder1( "Folder1" );
    wxString sSequence1( "Sequence1" );
    wxString sFile( "05.avi" );
    wxFileName filepath(mProjectFixture.TestFilesPath.GetFullPath(), sFile);

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