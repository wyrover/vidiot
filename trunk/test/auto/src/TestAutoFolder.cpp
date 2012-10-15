#include "TestAutoFolder.h"

#include <wx/ffile.h>
#include <wx/filefn.h>
#include <boost/thread.hpp>
#include "AutoFolder.h"
#include "FixtureGui.h"
#include "HelperApplication.h"
#include "HelperProjectView.h"
#include "HelperTestSuite.h"
#include "HelperWindow.h"
#include "UtilLogWxwidgets.h"

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
    waitForIdle();
    while ( model::AutoFolder::getSupportedFiles( tempdir.getFileName() ).size() < 1 )
    {
        pause(10);
    }
    ASSERT_EQUALS(countProjectView(), nDefaultItems + 2); // Added AutoFolder and the valid File

    // Clean up
    remove( autofolder1 );
}

} // namespace