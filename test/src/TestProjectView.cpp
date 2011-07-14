#include "TestProjectView.h"

#include <boost/assign/list_of.hpp>
#include "AutoFolder.h"
#include "File.h"
#include "FixtureGui.h"
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

    model::FolderPtr root = FixtureGui::createProject();
    model::FolderPtr autofolder1 = FixtureGui::addAutoFolder( path );
    model::FolderPtr folder1 = FixtureGui::addFolder( sFolder1 );
    model::SequencePtr sequence1 = FixtureGui::addSequence( sSequence1, folder1 );
    model::Files files1 = FixtureGui::addFiles( boost::assign::list_of(filepath), folder1 );

    ASSERT( autofolder1->getParent() == root );
    ASSERT( folder1->getParent() == root );
    ASSERT( sequence1->getParent() == folder1 );
    ASSERT( FixtureGui::countProjectView() == files.size() + 5); // +4: Root + Autofolder + Folder + Sequence + File node
    ASSERT( files1.size() == 1);
    ASSERT( files1.front()->getParent() == folder1 );
    ASSERT( root->find(sFile).size() == 1 );    // One file with a relative file name
    ASSERT( root->find(filepath.GetLongPath()).size() == 1 ); // And one file with an absolute file name

    FixtureGui::remove( files1.front() );
    ASSERT( FixtureGui::countProjectView() == files.size() + 4); // +4: Root + Autofolder + Folder + Sequence node
    FixtureGui::remove( folder1 ); // Also removes sequence1 which is contained in folder1
    ASSERT( FixtureGui::countProjectView() == files.size() + 2); // +4: Root + Autofolder node
}

void ProjectViewTests::testCreateSequence()
{
    LOG_DEBUG << "TEST_START";
    wxFileName path( "D:\\Vidiot\\test", "" );
    model::IPaths files = model::AutoFolder::getSupportedFiles( path );
    model::FolderPtr root = FixtureGui::createProject();
    model::FolderPtr autofolder1 = FixtureGui::addAutoFolder( path );
    model::SequencePtr sequence1 = FixtureGui::createSequence( autofolder1 );
    ASSERT( FixtureGui::countProjectView() == files.size() + 3); // +3: Root + Autofolder + Sequence + File node
    ASSERT( sequence1->getParent() == root);
}

} // namespace