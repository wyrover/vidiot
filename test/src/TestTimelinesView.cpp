#include "TestTimelinesView.h"

#include "ids.h"
#include "FixtureGui.h"
#include "Menu.h"
#include "Timeline.h"
#include "TimeLinesView.h"
#include "UtilList.h"
#include "UtilLog.h"
#include "Window.h"

namespace test {

void TestTimelinesView::testSequenceMenu()
{
    LOG_DEBUG << "TEST_START";
    wxString sFolder1( "Folder1" );
    wxString sSequence1( "Sequence1" );
    wxString sSequence2( "Sequence2" );

    model::FolderPtr root = FixtureGui::createProject();

    model::SequencePtr sequence1 = FixtureGui::addSequence( sSequence1 );
    ASSERT(FixtureGui::getTimeline(sequence1).getMenuHandler().getMenu() == gui::Window::get().GetMenuBar()->GetMenu(gui::Window::sSequenceMenuIndex));

    model::SequencePtr sequence2 = FixtureGui::addSequence( sSequence2 );
    ASSERT(FixtureGui::getSequenceMenu() == FixtureGui::getTimeline(sequence2).getMenuHandler().getMenu());

    FixtureGui::triggerMenu(ID_CLOSESEQUENCE);
    ASSERT(FixtureGui::getSequenceMenu() == FixtureGui::getTimeline(sequence1).getMenuHandler().getMenu());

    FixtureGui::triggerMenu(ID_CLOSESEQUENCE);
    ASSERT(FixtureGui::getSequenceMenu()->GetMenuItemCount() == 0); // When all sequences are closed, the default menu (member of Window) is shown, which is empty
}

} // namespace