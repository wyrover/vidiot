#include "TestAutoFolder.h"

#include <wx/ffile.h>
#include "FixtureProject.h"
#include "Folder.h"
#include "HelperApplication.h"
#include "HelperProjectView.h"
#include "HelperFileSystem.h"
#include "IPath.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestAutoFolder::testAddAutoFolder()
{
    StartTestSuite();

    model::FolderPtr root = createProject();
    ASSERT(root);
    model::IPaths InputFiles = getListOfInputFiles();

    {
        StartTest("Add empty autofolder to project view");
        RandomTempDir tempdir;
        model::FolderPtr autofolder1 = addAutoFolder( tempdir.getFileName() );
        WaitForChildCount(root, 2);
        remove( autofolder1 );
    }
    {
        StartTest("Add autofolder with one supported and one unsupported file to project view");
        RandomTempDir tempdir;

        wxFileName filepath(tempdir.getFileName().GetLongPath(),"invalid","avi");
        wxFFile aviFile1( filepath.GetLongPath(), "w" );
        aviFile1.Write( "Dummy Contents", wxFile::read_write );
        aviFile1.Close();

        wxString aviFileName = InputFiles.front()->getPath().GetLongPath();
        wxFileName filepath2(tempdir.getFileName().GetLongPath(),"valid","avi");
        bool copyok = wxCopyFile( aviFileName, filepath2.GetLongPath(), false );
        ASSERT(copyok);

        model::FolderPtr autofolder1 = addAutoFolder( tempdir.getFileName() );
        WaitForChildCount(root, 3);
        remove( autofolder1 );
    }
}

} // namespace