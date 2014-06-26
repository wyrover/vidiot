// Copyright 2013,2014 Eric Raijmakers.
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

#include "TestProjectView.h"

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
    wxFileName filepath(getTestFilesPath().GetFullPath(), sFile);

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