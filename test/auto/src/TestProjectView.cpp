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
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void ProjectViewTests::testAdditionAndRemoval()
{
    LOG_DEBUG << "TEST_START";

    wxFileName path( "D:\\Vidiot\\test", "" );
    wxString sFolder1( "Folder1" );
    wxString sSequence1( "Sequence1" );
    wxString sFile( "scene'20100102 12.32.48.avi" ); // Should be a file also in the autofolder
    wxFileName filepath = wxFileName( path.GetShortPath(), sFile );
    model::IPaths files = model::AutoFolder::getSupportedFiles( path );

    model::FolderPtr root = createProject();
    model::FolderPtr autofolder1 = addAutoFolder( path );
    model::FolderPtr folder1 = addFolder( sFolder1 );
    model::SequencePtr sequence1 = addSequence( sSequence1, folder1 );
    model::Files files1 = addFiles( boost::assign::list_of(filepath), folder1 );

    ASSERT_EQUALS(autofolder1->getParent(),root);
    ASSERT_EQUALS(folder1->getParent(),root);
    ASSERT_EQUALS(sequence1->getParent(),folder1);
    ASSERT_EQUALS(countProjectView(),files.size() + 5); // +4: Root + Autofolder + Folder + Sequence + File node
    ASSERT_EQUALS(files1.size(),1);
    ASSERT_EQUALS(files1.front()->getParent(),folder1);
    ASSERT_EQUALS(root->find(sFile).size(),1);    // One file with a relative file name
    ASSERT_EQUALS(root->find(filepath.GetLongPath()).size(),1); // And one file with an absolute file name

    remove( files1.front() );
    ASSERT_EQUALS(countProjectView(),files.size() + 4); // +4: Root + Autofolder + Folder + Sequence node
    remove( folder1 ); // Also removes sequence1 which is contained in folder1
    ASSERT_EQUALS(countProjectView(),files.size() + 2); // +4: Root + Autofolder node
}

void ProjectViewTests::testCreateSequence()
{
    LOG_DEBUG << "TEST_START";
    wxFileName path( "D:\\Vidiot\\test", "" );
    model::IPaths files = model::AutoFolder::getSupportedFiles( path );
    model::FolderPtr root = createProject();
    model::FolderPtr autofolder1 = addAutoFolder( path );
    model::SequencePtr sequence1 = createSequence( autofolder1 );
    ASSERT_EQUALS(countProjectView(),files.size() + 3); // +3: Root + Autofolder + Sequence + File node
    ASSERT_EQUALS(sequence1->getParent(),root);
}

} // namespace