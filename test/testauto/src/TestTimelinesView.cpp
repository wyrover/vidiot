// Copyright 2013-2016 Eric Raijmakers.
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

#include "TestTimelinesView.h"

namespace test {

void TestTimelinesView::testSequenceMenu()
{
    StartTestSuite();
    wxString sFolder1( "Folder1" );
    wxString sSequence1( "Sequence1" );
    wxString sSequence2( "Sequence2" );

    model::FolderPtr root = WindowCreateProject();

    model::SequencePtr sequence1 = ProjectViewAddSequence( sSequence1 );
    ASSERT_EQUALS(getSequenceMenu(), getTimeline(sequence1).getMenuHandler().getMenu());

    model::SequencePtr sequence2 = ProjectViewAddSequence( sSequence2 );
    ASSERT_EQUALS(getSequenceMenu(),getTimeline(sequence2).getMenuHandler().getMenu());

    WindowTriggerMenu(ID_CLOSESEQUENCE);
    ASSERT_EQUALS(getSequenceMenu(),getTimeline(sequence1).getMenuHandler().getMenu());

    WindowTriggerMenu(ID_CLOSESEQUENCE);
    ASSERT_ZERO(getSequenceMenu()->GetMenuItemCount()); // When all sequences are closed, the default menu (member of Window) is shown, which is empty
}
} // namespace