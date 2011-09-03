#include "TestAutoFolder.h"

#include <wx/ffile.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/thread.hpp>
#include "AutoFolder.h"
#include "FixtureApplication.h"
#include "HelperApplication.h"
#include "HelperWindow.h"
#include "HelperProjectView.h"
#include "UtilLog.h"
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
    LOG_DEBUG << "TEST_START";

    int nDefaultItems = countProjectView();

    // Make dir on disk
    wxFileName dirpath(wxFileName::GetTempDir(), "");
    dirpath.AppendDir(randomString(20));
    ASSERT(!wxDirExists(dirpath.GetLongPath()));
    dirpath.Mkdir();
    ASSERT(wxDirExists(dirpath.GetLongPath()));

    // Add autofolder to project view
    model::FolderPtr autofolder1 = addAutoFolder( dirpath );
    ASSERT_EQUALS(countProjectView(),nDefaultItems + 1); // Added Autofolder

    // Add file on disk
    wxFileName filepath(dirpath);
    filepath.SetFullName("test.avi");
    wxFFile aviFile1( filepath.GetLongPath(), "w" );
    aviFile1.Write( "Dummy Contents", wxFile::read_write );
    aviFile1.Close();
    
    // Wait until file addition seen. Loop is required to wait until the Watcher has seen the change
    waitForIdle();
    while ( model::AutoFolder::getSupportedFiles( dirpath ).size() < 1 )
    {
        pause(10);
    }
    ASSERT_EQUALS(countProjectView(), nDefaultItems + 2); // Added AutoFolder and File

    // Clean up
    remove( autofolder1 );
    bool removed = wxFileName::Rmdir( dirpath.GetLongPath(), wxPATH_RMDIR_RECURSIVE );
    ASSERT(removed);
}


} // namespace