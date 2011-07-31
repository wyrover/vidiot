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

void TestAutoFolder::testWatch()
{
    LOG_DEBUG << "TEST_START";

    wxString sFolder1( "Folder1" );
    wxString sFile1Contents( "test" );
    wxFileName sNewDir( "" );
    
    wxString tmp = wxFileName::GetTempDir ();
    wxFileName path( wxFileName::GetTempDir(), "");

    wxFileName dirpath(wxFileName::GetTempDir(), "");
    dirpath.AppendDir(randomString(20));
    wxFileName filepath(dirpath);
    filepath.SetFullName("test.avi");

    ASSERT(!wxDirExists(dirpath.GetLongPath()));
    dirpath.Mkdir();
    ASSERT(wxDirExists(dirpath.GetLongPath()));
    VAR_DEBUG(path);
    model::IPaths files = model::AutoFolder::getSupportedFiles(dirpath);

    model::FolderPtr root = createProject();
    model::FolderPtr autofolder1 = addAutoFolder( dirpath );
    ASSERT_EQUALS(countProjectView(),2); // Root + Autofolder

    wxString ff = filepath.GetLongPath();
    wxFFile aviFile1( filepath.GetLongPath(), "w" );
    aviFile1.Write( sFile1Contents, wxFile::read_write );
    aviFile1.Close();
    
    waitForIdle();
    // This loop is required to wait until the Watcher has seen the change
    // Not the best solution......
    while ( model::AutoFolder::getSupportedFiles( dirpath ).size() < 1 )
    {
        pause(10);
    }
    ASSERT_EQUALS(countProjectView(),3); // Root + Autofolder

    remove( autofolder1 );
    bool removed = wxFileName::Rmdir( dirpath.GetLongPath(), wxPATH_RMDIR_RECURSIVE );
    ASSERT(removed);
    waitForIdle();
}


} // namespace