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

// TODO replace wxfilename with boost::filesystem everywhere
void TestTimelinesView::testSequenceMenu()
{
    wxString sFolder1( "Folder1" );
    wxString sSequence1( "Sequence1" );
    wxString sSequence2( "Sequence2" );

    model::FolderPtr root = FixtureGui::createProject();

    model::SequencePtr sequence1 = FixtureGui::addSequence( sSequence1 );
    ASSERT(getTimeline(sequence1)->getMenuHandler().getMenu() == gui::Window::get().GetMenuBar()->GetMenu(gui::Window::sSequenceMenuIndex));

    model::SequencePtr sequence2 = FixtureGui::addSequence( sSequence2 );
    ASSERT(getSequenceMenu() == getTimeline(sequence2)->getMenuHandler().getMenu());

    FixtureGui::triggerMenu(ID_CLOSESEQUENCE);
    ASSERT(getSequenceMenu() == getTimeline(sequence1)->getMenuHandler().getMenu());

    FixtureGui::triggerMenu(ID_CLOSESEQUENCE);
    ASSERT(getSequenceMenu()->GetMenuItemCount() == 0); // When all sequences are closed, the default menu (member of Window) is shown, which is empty
}

// static
wxMenu* TestTimelinesView::getSequenceMenu()
{
    return gui::Window::get().GetMenuBar()->GetMenu(gui::Window::sSequenceMenuIndex);
}

// static
gui::timeline::Timeline* TestTimelinesView::getTimeline( model::SequencePtr sequence )
{
    return gui::TimelinesView::get().findPage(sequence).second;
}

} // namespace