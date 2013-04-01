#include "TestAutoFolder.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestAutoFolder::setUp()
{
    mProjectFixture.init();
}

void TestAutoFolder::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

//RUNONLY(testWatch);
void TestAutoFolder::testWatch()
{
    StartTestSuite();

    int nDefaultItems = countProjectView();

    // Add autofolder to project view
    RandomTempDir tempdir;
    model::FolderPtr autofolder1 = addAutoFolder( tempdir.getFileName() );
    ASSERT_EQUALS(countProjectView(),nDefaultItems + 1); // Added Autofolder

    // Add supported but not valid file on disk
    wxFileName filepath(tempdir.getFileName().GetLongPath(),"invalid","avi");
    wxFFile aviFile1( filepath.GetLongPath(), "w" );
    aviFile1.Write( "Dummy Contents", wxFile::read_write );
    aviFile1.Close();

    // Add supported and valid file on disk
    wxString aviFileName = mProjectFixture.InputFiles.front()->getPath().GetLongPath();
    filepath.SetFullName("valid.avi");
    bool copyok = wxCopyFile( aviFileName, filepath.GetLongPath(), false );
    ASSERT(copyok);

    // Wait until file addition seen. Loop is required to wait until the Watcher has seen the valid file
    //waitForIdle();
    //while ( getSupportedFiles( tempdir.getFileName() ).size() < 1 )
    //{
    //    pause(10);
    //}
    gui::Worker::get().waitUntilQueueEmpty(); // This assumes that this wait is started before the worker actually does the work. Racer.
//    waitForIdle();

    model::NodePtrs nodes = mProjectFixture.mRoot->find(autofolder1->getName());
    ASSERT_MORE_THAN_ZERO(nodes.size());
    MoveOnScreen(Center(nodes.front()));
    wxUIActionSimulator().MouseClick(); // Select the auto folder
    waitForIdle();
    Type(WXK_RIGHT); // Open the auto folder

    ASSERT_EQUALS(countProjectView(), nDefaultItems + 2); // Added AutoFolder and the valid File

    // Clean up
    remove( autofolder1 );
}

} // namespace