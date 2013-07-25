#include "TestStillImage.h"

#include "AudioClip.h"
#include "AudioTrack.h"
#include "Config.h"
#include "CreateTransition.h"
#include "Details.h"
#include "DetailsClip.h"
#include "DetailsTrim.h"
#include "Drag.h"
#include "EmptyClip.h"
#include "ExecuteDrop.h"
#include "HelperApplication.h"
#include "HelperProjectView.h"
#include "HelperTimeline.h"
#include "HelperTimelineAssert.h"
#include "HelperTimelineDrag.h"
#include "HelperTimelineTrim.h"
#include "HelperTimelinesView.h"
#include "HelperTransition.h"
#include "HelperWindow.h"
#include "IClip.h"
#include "ids.h"
#include "Layout.h"
#include "ProjectViewCreateAutoFolder.h"
#include "ProjectViewCreateSequence.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "Transition.h"
#include "TrimClip.h"
#include "VideoClip.h"
#include "VideoTrack.h"
#include "Zoom.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestStillImage::setUp()
{
    mProjectFixture.init();
}

void TestStillImage::tearDown()
{
    mProjectFixture.destroy();
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestStillImage::testUndo()
{
    StartTestSuite();

    wxString sFolder1( "Folder1" );
    model::FolderPtr folder1 = addFolder( sFolder1 );
    wxFileName TestFilesPath = wxFileName(SOURCE_ROOT,"");
    TestFilesPath.AppendDir("test");
    TestFilesPath.AppendDir("filetypes_image");
    ASSERT(TestFilesPath.IsDir());
    ASSERT(TestFilesPath.DirExists());
    TestFilesPath.SetFullName("Laney -6th best amp.jpg");
    model::Files files1 = addFiles( boost::assign::list_of(TestFilesPath.GetFullPath()), folder1 );
    model::FilePtr imageFile = files1.front();

    DragFromProjectViewToTimeline( imageFile,  getTimeline().GetScreenPosition() - getTimeline().getScrolling().getOffset()  + wxPoint(HCenter(VideoClip(0,4)), VCenter(VideoTrack(0))) );

    Scrub(HCenter(VideoClip(0,5)),HCenter(VideoClip(0,5)) + 10);

    Click(Center(VideoClip(0,5)));
    Type(WXK_DELETE);

    Undo();

    Scrub(HCenter(VideoClip(0,5)),HCenter(VideoClip(0,5)) + 10);
}

} // namespace