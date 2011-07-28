#include "TestTimelinesView.h"

#include "ids.h"
#include "FixtureGui.h"
#include "HelperWindow.h"
#include "HelperProjectView.h"
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

    model::FolderPtr root = createProject();

    model::SequencePtr sequence1 = addSequence( sSequence1 );
    ASSERT(getTimeline(sequence1).getMenuHandler().getMenu() == gui::Window::get().GetMenuBar()->GetMenu(gui::Window::sSequenceMenuIndex));

    model::SequencePtr sequence2 = addSequence( sSequence2 );
    ASSERT(getSequenceMenu() == getTimeline(sequence2).getMenuHandler().getMenu());

    triggerMenu(ID_CLOSESEQUENCE);
    ASSERT(getSequenceMenu() == getTimeline(sequence1).getMenuHandler().getMenu());

    triggerMenu(ID_CLOSESEQUENCE);
    ASSERT(getSequenceMenu()->GetMenuItemCount() == 0); // When all sequences are closed, the default menu (member of Window) is shown, which is empty
}

} // namespace