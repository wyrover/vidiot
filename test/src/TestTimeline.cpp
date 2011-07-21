#include "TestTimeline.h"

#include <boost/foreach.hpp>
#include "FixtureGui.h"
#include "Menu.h"
#include "IClip.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "Timeline.h"
#include "ClipView.h"
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

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

void TestTimeline::setUp()
{
    model::FolderPtr root = FixtureGui::createProject();
    model::FolderPtr autofolder1 = FixtureGui::addAutoFolder( TestFilesPath );
    model::SequencePtr sequence1 = FixtureGui::createSequence( autofolder1 );
}

void TestTimeline::tearDown()
{
}

//////////////////////////////////////////////////////////////////////////
// TEST CASES
//////////////////////////////////////////////////////////////////////////

void TestTimeline::testSelection()
{
    LOG_DEBUG << "TEST_START";

    const model::IClips& clips = FixtureGui::getActiveSequence()->getVideoTrack(0)->getClips();

    int nClips = NumberOfVideoClipsInTrack(0);

    // Test CTRL clicking all clips one by one
    KeyDown(0, wxMOD_CONTROL);
    BOOST_FOREACH(model::IClipPtr clip, clips)
    {
        click(clip);
    }
    KeyUp(0, wxMOD_CONTROL);
    ASSERT_SELECTION_SIZE(InputFiles.size());
    FixtureGui::getTimeline().getSelection().unselectAll();
    ASSERT_SELECTION_SIZE(0);

    // Test SHIFT clicking the entire list
    KeyDown(0, wxMOD_SHIFT);
    click(clips.front());
    click(clips.back());
    KeyUp(0, wxMOD_SHIFT);
    ASSERT_SELECTION_SIZE(InputFiles.size());

    // Test SHIFT clicking only the partial list
    FixtureGui::getTimeline().getSelection().unselectAll();
    ASSERT_SELECTION_SIZE(0);
    click(VideoClip(0,2));
    KeyDown(0, wxMOD_SHIFT);
    click(VideoClip(0,4));
    KeyUp(0, wxMOD_SHIFT);
    ASSERT_SELECTION_SIZE(3);

    // Test (de)selecting one clip with CTRL click
    KeyDown(0, wxMOD_CONTROL);
    click(VideoClip(0,3));
    KeyUp(0, wxMOD_CONTROL);
    ASSERT_SELECTION_SIZE(2);
    FixtureGui::waitForIdle();
    KeyDown(0, wxMOD_CONTROL);
    click(VideoClip(0,3));
    KeyUp(0, wxMOD_CONTROL);
    ASSERT_SELECTION_SIZE(3);
}

void TestTimeline::testTransition()
{
    LOG_DEBUG << "TEST_START";

    // Give focus to timeline
    MouseMove(TimelinePosition() + wxPoint(LeftPixel(VideoClip(0,2)), TopPixel(VideoClip(0,2))));
    MouseClick();

    // Zoom in maximally. This is required to have accurate pointer positioning further on.
    // Without this, truncating integers in ptsToPixels and pixelsToPts causes wrong pointer placement.
    KeyDown(0, wxMOD_CONTROL);
    Char('=');
    KeyUp(0, wxMOD_CONTROL);
    FixtureGui::waitForIdle();

    // Shift Trim right clip
    pixel x = LeftPixel(VideoClip(0,2)) + 1; // The +1 is required to fix errors where the pointer is moved to a slightly different position (don't know why exactly)
    pixel y = TopPixel(VideoClip(0,2)) + 10;
    MouseMove(TimelinePosition() + wxPoint(x, y));
    KeyDown(0, wxMOD_SHIFT);
    MouseDown();
    MouseMove(TimelinePosition() + wxPoint(x + 50, y));
    MouseUp();
    KeyUp(0, wxMOD_SHIFT);
    FixtureGui::waitForIdle();

    // Shift Trim left clip
    pixel xr = RightPixel(VideoClip(0,1)); // The +1 is required to fix errors where the pointer is moved to a slightly different position (don't know why exactly)
    pixel yr = TopPixel(VideoClip(0,1)) + 10;
    MouseMove(TimelinePosition() + wxPoint(xr, yr));
    KeyDown(0, wxMOD_SHIFT);
    MouseDown();
    MouseMove(TimelinePosition() + wxPoint(xr - 50, yr));
    MouseUp();
    KeyUp(0, wxMOD_SHIFT);
    FixtureGui::waitForIdle();

    // Determine length of second and third clips
    pts secondClipLength = VideoClip(0,1)->getLength();
    pts thirdClipLength = VideoClip(0,2)->getLength();

    // Create crossfade
    ASSERT(getNonEmptyClipsCount() == InputFiles.size() * 2 );
    Char('c');
    FixtureGui::waitForIdle();
    ASSERT(getNonEmptyClipsCount() == InputFiles.size() * 2 + 1); // Transition added
    pts secondClipLengthWithTransition = VideoClip(0,1)->getLength(); // Counting is 0-based, 1 -> clip 2
    pts thirdClipLengthWithTransition  = VideoClip(0,3)->getLength(); // Clip 3 has become index 3 due to addition of transition (counting is 0-based)

    // Delete clip after the crossfade
    click(VideoClip(0,3)); // Clip 3 has become index 3 due to addition of transition (counting is 0-based)
    ASSERT_SELECTION_SIZE(1);
    KeyDown(WXK_DELETE);
    KeyUp(WXK_DELETE);
    FixtureGui::waitForIdle();
    ASSERT(getNonEmptyClipsCount() == InputFiles.size() * 2 - 2); // Clip and link and transition removed
    ASSERT(secondClipLength == VideoClip(0,1)->getLength()); // Original length of second clip must be restored

    FixtureGui::triggerUndo(); // Trigger undo of delete
    ASSERT(secondClipLengthWithTransition == VideoClip(0,1)->getLength());
    ASSERT(thirdClipLengthWithTransition  == VideoClip(0,3)->getLength()); // Clip 3 has become index 3 due to addition of transition (counting is 0-based)

    // Delete clip before the crossfade
    click(VideoClip(0,1));
    ASSERT_SELECTION_SIZE(1);
    KeyDown(WXK_DELETE);
    KeyUp(WXK_DELETE);
    FixtureGui::waitForIdle();
    ASSERT(getNonEmptyClipsCount() == InputFiles.size() * 2 - 2); // Clip and link and transition removed
    ASSERT(thirdClipLength == VideoClip(0,2)->getLength()); // Original length of third clip must be restored

    FixtureGui::triggerUndo(); // Trigger undo of delete
    ASSERT(secondClipLengthWithTransition == VideoClip(0,1)->getLength());
    ASSERT(thirdClipLengthWithTransition  == VideoClip(0,3)->getLength()); // Clip 3 has become index 3 due to addition of transition (counting is 0-based)

    pixel top = TopPixel(VideoClip(0,2)) - 5;
    pixel left = LeftPixel(VideoClip(0,2)) - 1;
    pixel right = RightPixel(VideoClip(0,2)) + 1;
    MouseMove(TimelinePosition() + wxPoint(left,top));
    MouseDown();
    for (int i = left; i < right; ++i)
    {
        MouseMove(TimelinePosition() + wxPoint(i,top));
        FixtureGui::waitForIdle();
    }
    MouseUp();
}

void TestTimeline::testDnd()
{
    LOG_DEBUG << "TEST_START";
}

} // namespace