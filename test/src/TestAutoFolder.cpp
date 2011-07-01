#include "TestAutoFolder.h"

#include <wx/ffile.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/thread.hpp>
#include "AutoFolder.h"
#include "FixtureGui.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"

namespace test {

// TODO replace boost filename with wxfilename everywhere
    // TODO convert all paths to generic types and long forms
void TestAutoFolder::testWatch()
{
    LOG_DEBUG << "TEST_START";
    wxString sFolder1( "Folder1" );
    wxString sFile1Contents( "test" );
    wxFileName sNewDir( "" );
    
    // todo ensure that in this pasth a ~1 folder is used to test expansion to long form

    wxString tmp = wxFileName::GetTempDir ();
    wxFileName path( wxFileName::GetTempDir(), "");

    wxFileName dirpath(wxFileName::GetTempDir(), "");
    dirpath.AppendDir( FixtureGui::randomString(20) ); // todo uniqueify
    wxFileName filepath( dirpath );
    filepath.SetFullName( "test.avi" );

    ASSERT( !wxDirExists( dirpath.GetLongPath() ) );
    dirpath.Mkdir();
    ASSERT( wxDirExists( dirpath.GetLongPath() ) );
    VAR_DEBUG(path);
    model::IPaths files = model::AutoFolder::getSupportedFiles( dirpath );

    model::FolderPtr root = FixtureGui::createProject();
    model::FolderPtr autofolder1 = FixtureGui::addAutoFolder( dirpath );
    ASSERT( FixtureGui::countProjectView() == 2); // Root + Autofolder

    wxString ff = filepath.GetLongPath();
    wxFFile aviFile1( filepath.GetLongPath(), "w" );
    aviFile1.Write( sFile1Contents, wxFile::read_write );
    aviFile1.Close();
    
    FixtureGui::waitForIdle();
    // This loop is required to wait until the FSwatcher has seen the change
    // Not the best solution......
    while ( model::AutoFolder::getSupportedFiles( dirpath ).size() < 1 )
    {
        boost::this_thread::sleep(boost::posix_time::milliseconds(10));
    }
    ASSERT( FixtureGui::countProjectView() == 3 ); // Root + Autofolder

    FixtureGui::remove( autofolder1 );
    bool removed = wxFileName::Rmdir( dirpath.GetLongPath(), wxPATH_RMDIR_RECURSIVE );
    ASSERT(removed);
    FixtureGui::waitForIdle();
}


} // namespace