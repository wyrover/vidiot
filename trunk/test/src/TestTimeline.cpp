#include "TestTimeline.h"

#include <wx/uiaction.h>
#include <boost/foreach.hpp>
#include "FixtureGui.h"
#include "Menu.h"
#include "IClip.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "Timeline.h"
#include "ClipView.h"
#include "AutoFolder.h"
#include "TimeLinesView.h"
#include "PositionInfo.h"
#include "UtilLogWxwidgets.h"
#include "Track.h"
#include "UtilList.h"
#include "ViewMap.h"
#include "Selection.h"
#include "UtilLog.h"
#include "Window.h"
#include "Zoom.h"

namespace test {

void click(gui::timeline::Timeline& timeline, model::IClipPtr clip)
{
    // yposition
    pixel trackY = FixtureGui::getTimeline().getSequenceView().getPosition(clip->getTrack());
    pixel trackH = clip->getTrack()->getHeight();
    pixel clickY = trackY + (trackH / 2);

    // xposition
    gui::timeline::ClipView* view = timeline.getViewMap().getView(clip);
    pixel clickX = (view->getLeftPixel() + view->getRightPixel()) / 2;
   
    wxUIActionSimulator().MouseMove(timeline.GetScreenPosition() + wxPoint(clickX, clickY));
    wxUIActionSimulator().MouseClick();
    FixtureGui::waitForIdle();
}

void ASSERT_SELECTION_SIZE(int size)
{
    ASSERT(FixtureGui::getSelectedClipsCount() == 2 * size); // * 2 since AudioClips are selected also
}

void TestTimeline::testSelection()
{
    LOG_DEBUG << "TEST_START";
    wxUIActionSimulator simu;
    wxFileName path( "D:\\Vidiot\\test", "" );
    model::IPaths files = model::AutoFolder::getSupportedFiles( path );
    model::FolderPtr root = FixtureGui::createProject();
    model::FolderPtr autofolder1 = FixtureGui::addAutoFolder( path );
    model::SequencePtr sequence1 = FixtureGui::createSequence( autofolder1 );
    gui::timeline::Timeline& timeline = FixtureGui::getTimeline(sequence1);

    const model::IClips& clips = sequence1->getVideoTrack(0)->getClips();

    int nClips = FixtureGui::getNumberOfClipsInVideoTrack(0);

    // Test CTRL clicking all clips one by one
    wxUIActionSimulator().KeyDown(0, wxMOD_CONTROL);
    BOOST_FOREACH(model::IClipPtr clip, clips)
    {
        click(timeline,clip);
    }
    wxUIActionSimulator().KeyUp(0, wxMOD_CONTROL);
    ASSERT_SELECTION_SIZE(files.size());
    timeline.getSelection().unselectAll();
    ASSERT_SELECTION_SIZE(0);

    // Test SHIFT clicking the entire list
    wxUIActionSimulator().KeyDown(0, wxMOD_SHIFT);
    click(timeline,clips.front());
    click(timeline,clips.back());
    wxUIActionSimulator().KeyUp(0, wxMOD_SHIFT);
    ASSERT_SELECTION_SIZE(files.size());

    // Test SHIFT clicking only the partial list
    timeline.getSelection().unselectAll();
    ASSERT_SELECTION_SIZE(0);
    click(timeline, FixtureGui::getVideoClip(0,2));
    wxUIActionSimulator().KeyDown(0, wxMOD_SHIFT);
    click(timeline, FixtureGui::getVideoClip(0,4));
    wxUIActionSimulator().KeyUp(0, wxMOD_SHIFT);
    ASSERT_SELECTION_SIZE(3);

    // Test (de)selecting one clip with CTRL click
    wxUIActionSimulator().KeyDown(0, wxMOD_CONTROL);
    click(timeline, FixtureGui::getVideoClip(0,3));
    wxUIActionSimulator().KeyUp(0, wxMOD_CONTROL);
    ASSERT_SELECTION_SIZE(2);
    FixtureGui::waitForIdle();
    wxUIActionSimulator().KeyDown(0, wxMOD_CONTROL);
    click(timeline, FixtureGui::getVideoClip(0,3));
    wxUIActionSimulator().KeyUp(0, wxMOD_CONTROL);
    ASSERT_SELECTION_SIZE(3);
 }

void TestTimeline::testTransition()
{
    LOG_DEBUG << "TEST_START";
    wxUIActionSimulator simu;
    wxFileName path( "D:\\Vidiot\\test", "" );
    model::IPaths files = model::AutoFolder::getSupportedFiles( path );
    model::FolderPtr root = FixtureGui::createProject();
    model::FolderPtr autofolder1 = FixtureGui::addAutoFolder( path );
    model::SequencePtr sequence1 = FixtureGui::createSequence( autofolder1 );
    gui::timeline::Timeline& timeline = FixtureGui::getTimeline(sequence1);
    model::IClipPtr thirdClip = sequence1->getVideoTrack(0)->getClipByIndex(2);

    // Give focus to timeline
    wxUIActionSimulator().MouseMove(timeline.GetScreenPosition() + wxPoint(FixtureGui::getLeft(thirdClip), FixtureGui::getTop(thirdClip)));
    wxUIActionSimulator().MouseClick();

    // Zoom in maximally. This is required to have accurate pointer positioning further on.
    // Without this, truncating integers in ptsToPixels and pixelsToPts causes wrong pointer placement.
    wxUIActionSimulator().KeyDown(0, wxMOD_CONTROL);
    wxUIActionSimulator().Char('=');
    wxUIActionSimulator().KeyUp(0, wxMOD_CONTROL);
    FixtureGui::waitForIdle();

    // Shift Trim right clip
    pixel x = FixtureGui::getLeft(thirdClip) + 1; // The +1 is required to fix errors where the pointer is moved to a slightly different position (don't know why exactly)
    pixel y = FixtureGui::getTop(thirdClip) + 10;
    wxUIActionSimulator().MouseMove(timeline.GetScreenPosition() + wxPoint(x, y));
    wxUIActionSimulator().KeyDown(0, wxMOD_SHIFT);
    wxUIActionSimulator().MouseDown();
    wxUIActionSimulator().MouseMove(timeline.GetScreenPosition() + wxPoint(x + 50, y));
    wxUIActionSimulator().MouseUp();
    wxUIActionSimulator().KeyUp(0, wxMOD_SHIFT);
    FixtureGui::waitForIdle();

    // Shift Trim left clip
    model::IClipPtr secondClip = sequence1->getVideoTrack(0)->getClipByIndex(1);
    pixel xr = FixtureGui::getRight(secondClip); // The +1 is required to fix errors where the pointer is moved to a slightly different position (don't know why exactly)
    pixel yr = FixtureGui::getTop(secondClip) + 10;
    wxUIActionSimulator().MouseMove(timeline.GetScreenPosition() + wxPoint(xr, yr));
    wxUIActionSimulator().KeyDown(0, wxMOD_SHIFT);
    wxUIActionSimulator().MouseDown();
    wxUIActionSimulator().MouseMove(timeline.GetScreenPosition() + wxPoint(x - 50, y));
    wxUIActionSimulator().MouseUp();
    wxUIActionSimulator().KeyUp(0, wxMOD_SHIFT);
    FixtureGui::waitForIdle();

    // Determine length of second and third clips
    pts secondClipLength = FixtureGui::getVideoClip(0,1)->getLength();
    pts thirdClipLength = FixtureGui::getVideoClip(0,2)->getLength();

    // Create crossfade
    ASSERT(FixtureGui::getNonEmptyClipsCount() == files.size() * 2 );
    wxUIActionSimulator().Char('c');
    FixtureGui::waitForIdle();
    ASSERT(FixtureGui::getNonEmptyClipsCount() == files.size() * 2 + 1); // Transition added
    pts secondClipLengthWithTransition = FixtureGui::getVideoClip(0,1)->getLength(); // Counting is 0-based, 1 -> clip 2
    pts thirdClipLengthWithTransition  = FixtureGui::getVideoClip(0,3)->getLength(); // Clip 3 has become index 3 due to addition of transition (counting is 0-based)

    // Delete clip after the crossfade
    click(timeline, FixtureGui::getVideoClip(0,3)); // Clip 3 has become index 3 due to addition of transition (counting is 0-based)
    ASSERT_SELECTION_SIZE(1);
    wxUIActionSimulator().KeyDown(WXK_DELETE);
    wxUIActionSimulator().KeyUp(WXK_DELETE);
    FixtureGui::waitForIdle();
    ASSERT(FixtureGui::getNonEmptyClipsCount() == files.size() * 2 - 2); // Clip and link and transition removed
    ASSERT(secondClipLength == FixtureGui::getVideoClip(0,1)->getLength()); // Original length of second clip must be restored
    
    FixtureGui::triggerUndo(); // Trigger undo of delete
    ASSERT(secondClipLengthWithTransition == FixtureGui::getVideoClip(0,1)->getLength());
    ASSERT(thirdClipLengthWithTransition  == FixtureGui::getVideoClip(0,3)->getLength()); // Clip 3 has become index 3 due to addition of transition (counting is 0-based)

    // Delete clip before the crossfade
    click(timeline, FixtureGui::getVideoClip(0,1));
    ASSERT_SELECTION_SIZE(1);
    wxUIActionSimulator().KeyDown(WXK_DELETE);
    wxUIActionSimulator().KeyUp(WXK_DELETE);
    FixtureGui::waitForIdle();
    ASSERT(FixtureGui::getNonEmptyClipsCount() == files.size() * 2 - 2); // Clip and link and transition removed
    ASSERT(thirdClipLength == FixtureGui::getVideoClip(0,2)->getLength()); // Original length of third clip must be restored
    
    FixtureGui::triggerUndo(); // Trigger undo of delete
    ASSERT(secondClipLengthWithTransition == FixtureGui::getVideoClip(0,1)->getLength());
    ASSERT(thirdClipLengthWithTransition  == FixtureGui::getVideoClip(0,3)->getLength()); // Clip 3 has become index 3 due to addition of transition (counting is 0-based)

    pixel top = FixtureGui::getTop(FixtureGui::getVideoClip(0,2)) - 5;
    pixel left = FixtureGui::getLeft(FixtureGui::getVideoClip(0,2)) - 1;
    pixel right = FixtureGui::getRight(FixtureGui::getVideoClip(0,2)) + 1;
    wxUIActionSimulator().MouseMove(timeline.GetScreenPosition() + wxPoint(left,top));
    wxUIActionSimulator().MouseDown();
    for (int i = left; i < right; ++i)
    {
        wxUIActionSimulator().MouseMove(timeline.GetScreenPosition() + wxPoint(i,top));
        FixtureGui::waitForIdle();
    }
    wxUIActionSimulator().MouseUp();
    FixtureGui::pause();
}

void TestTimeline::testDnd()
{
    LOG_DEBUG << "TEST_START";
}

} // namespace