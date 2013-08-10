// Copyright 2013 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

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