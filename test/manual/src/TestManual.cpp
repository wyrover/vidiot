#include "TestManual.h"

#include "Dialog.h"
#include "FixtureGui.h"
#include "HelperApplication.h"
#include "HelperModel.h"
#include "HelperProjectView.h"
#include "HelperTestSuite.h"
#include "HelperTimeline.h"
#include "HelperTimelinesView.h"
#include "HelperWindow.h"
#include "ProjectView.h"
#include "ids.h"
#include "Layout.h"
#include "File.h"
#include "Logging.h"
#include "Scrolling.h"
#include "Sequence.h"
#include "Timeline.h"
#include "Track.h"
#include "UtilLog.h"
#include "VideoTrack.h"
#include "UtilLogWxwidgets.h"
#include <boost/foreach.hpp>
#include <wx/uiaction.h>

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestManual::setUp()
{
    mProjectFixture.init();
}

void TestManual::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestManual::testManual()
{
    StartTestSuite();
    StartTest("Manual...");
    LOG_ERROR << dump(getSequence());
    pause(60 * 500000);

    ClickTopLeft(&getProjectView(), wxPoint(10,getProjectView().getHeaderHeight() / 2));
    waitForIdle();

    wxString sSequence( "00BlankSequence" );
    model::SequencePtr mSequence = addSequence( sSequence,  mProjectFixture.mRoot);
    wxString sFolder1( "00BlankFolder" );
    model::FolderPtr folder1 = addFolder( sFolder1 );

    wxFileName filepath = wxFileName(SOURCE_ROOT,"");
    filepath.AppendDir("test");
    filepath.SetName("MVI_0512");
    filepath.SetExt("mov");
    ASSERT(filepath.Exists());

    model::Files files1 = addFiles( boost::assign::list_of(filepath.GetFullPath()), folder1 );

    DragFromProjectViewToTimeline( files1.front(),  getTimeline().GetScreenPosition() - getTimeline().getScrolling().getOffset()  + wxPoint(3, VCenter(VideoTrack(0))) );

    pause(60000 * 60); // One hour should be enough...
}

} // namespace