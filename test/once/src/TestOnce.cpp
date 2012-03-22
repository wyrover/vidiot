#include "TestOnce.h"

#include <wx/uiaction.h>
#include "typeinfo.h"

#include <wx/gdicmn.h>
#include <boost/foreach.hpp>
#include "AudioClip.h"
#include "AudioTrack.h"
#include "AutoFolder.h"
#include "ClipView.h"
#include "Config.h"
#include "CreateTransition.h"
#include "DeleteSelectedClips.h"
#include "Dialog.h"
#include "Dump.h"
#include "EmptyClip.h"
#include "ExecuteDrop.h"
#include "FixtureApplication.h"
#include "HelperApplication.h"
#include "HelperProjectView.h"
#include "HelperTimeline.h"
#include "HelperTimelineAssert.h"
#include "HelperTimelinesView.h"
#include "HelperTransition.h"
#include "HelperWindow.h"
#include "IClip.h"
#include "Intervals.h"
#include "Layout.h"
#include "MousePointer.h"
#include "PositionInfo.h"
#include "ProjectViewCreateAutoFolder.h"
#include "ProjectViewCreateSequence.h"
#include "Selection.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "Timeline.h"
#include "Track.h"
#include "Transition.h"
#include "Trim.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "VideoClip.h"
#include "VideoTrack.h"
#include "VideoTransition.h"
#include "ViewMap.h"
#include "Window.h"
#include "Zoom.h"
#include "ids.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestOnce::setUp()
{
    mProjectFixture.init();
}

void TestOnce::tearDown()
{
    mProjectFixture.destroy();
}

auto PrepareSnapping = [](bool enableSnapping)
{
    checkMenu(ID_SNAP_CLIPS, enableSnapping);
    checkMenu(ID_SNAP_CURSOR, enableSnapping);
    DeselectAllClips();
};

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestOnce::testOnce()
{
    //BREAK();
    getTimeline().getDump().dump();
    //    PrepareSnapping(true);
    StartTestSuite();
    Zoom level(2);
    {
        StartTest("Trim: EndTrim: Enlarge the last clip in a track (there is no empty clip after it anymore)");
         TrimRight(VideoClip(0,3),40,false);
         pts length = VideoClip(0,3)->getLength();
         DragAlignLeft(Center(VideoClip(0,3)),RightPixel(VideoClip(0,7)));
         TrimRight(VideoClip(0,7),-20,false);
         ASSERT_MORE_THAN(VideoClip(0,7)->getLength(), length);
         Undo();
         Undo();
         Undo();

    }

       //wxString sFile( "scene'20100102 12.32.48.avi" ); // Should be a file also in the autofolder
    // model::NodePtr file = mProjectFixture.mAutoFolder->find(sFile).front();
    // wxPoint root = findNode(mProjectFixture.mRoot);
    // wxPoint from = findNode(file);
    // MoveWithinWidget(from,getProjectView().GetScreenPosition());
    // DragFromProjectViewToTimeline(from, Center(VideoClip(0,2)));
    // todo snapto also for trimming

    // todo bug: make intransition/outtransition and then drag inwards: transition is removed!

}

} // namespace