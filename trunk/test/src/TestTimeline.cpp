#include "TestTimeline.h"

#include <boost/foreach.hpp>
#include "ClipView.h"
#include "EmptyClip.h"
#include "FixtureGui.h"
#include "IClip.h"
#include "Menu.h"
#include "PositionInfo.h"
#include "Selection.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "Timeline.h"
#include "TimeLinesView.h"
#include "Track.h"
#include "Transition.h"
#include "UtilList.h"
#include "UtilLog.h"
#include "UtilLogWxwidgets.h"
#include "ViewMap.h"
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
        Click(clip);
    }
    KeyUp(0, wxMOD_CONTROL);
    ASSERT_SELECTION_SIZE(InputFiles.size());
    FixtureGui::getTimeline().getSelection().unselectAll();
    ASSERT_SELECTION_SIZE(0);

    // Test SHIFT clicking the entire list
    KeyDown(0, wxMOD_SHIFT);
    Click(clips.front());
    Click(clips.back());
    KeyUp(0, wxMOD_SHIFT);
    ASSERT_SELECTION_SIZE(InputFiles.size());

    // Test SHIFT clicking only the partial list
    FixtureGui::getTimeline().getSelection().unselectAll();
    ASSERT_SELECTION_SIZE(0);
    Click(VideoClip(0,2));
    KeyDown(0, wxMOD_SHIFT);
    Click(VideoClip(0,4));
    KeyUp(0, wxMOD_SHIFT);
    ASSERT_SELECTION_SIZE(3);

    // Test (de)selecting one clip with CTRL click
    KeyDown(0, wxMOD_CONTROL);
    Click(VideoClip(0,3));
    KeyUp(0, wxMOD_CONTROL);
    ASSERT_SELECTION_SIZE(2);
    FixtureGui::waitForIdle();
    KeyDown(0, wxMOD_CONTROL);
    Click(VideoClip(0,3));
    KeyUp(0, wxMOD_CONTROL);
    ASSERT_SELECTION_SIZE(3);

    // Test selection the transition between two clips when shift selecting
    FixtureGui::getTimeline().getSelection().unselectAll();
    TrimLeft(VideoClip(0,2),30,true);
    TrimRight(VideoClip(0,1),30,true);
    Char('c');
    FixtureGui::waitForIdle();
    Click(VideoClip(0,1));
    KeyDown(0, wxMOD_SHIFT);
    Click(VideoClip(0,3));
    KeyUp(0, wxMOD_SHIFT);
    ASSERT(VideoClip(0,2)->isA<model::Transition>() && VideoClip(0,2)->getSelected());
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

    // Shift Trim clips to make room for transition
    TrimLeft(VideoClip(0,2),50,true);
    TrimRight(VideoClip(0,1),50,true);

    // Determine length of second and third clips
    pts secondClipLength = VideoClip(0,1)->getLength();
    pts thirdClipLength = VideoClip(0,2)->getLength();

    // Create crossfade
    ASSERT(getNonEmptyClipsCount() == InputFiles.size() * 2 );
    Char('c');
    FixtureGui::waitForIdle();
    ASSERT(getNonEmptyClipsCount() == InputFiles.size() * 2 + 1); // Transition added
    model::TransitionPtr transition = boost::dynamic_pointer_cast<model::Transition>(VideoClip(0,2));
    ASSERT(transition);
    ASSERT(transition->getLeft() > 0)(transition);
    ASSERT(transition->getRight() > 0)(transition);

    pts secondClipLengthWithTransition = VideoClip(0,1)->getLength(); // Counting is 0-based, 1 -> clip 2
    pts thirdClipLengthWithTransition  = VideoClip(0,3)->getLength(); // Clip 3 has become index 3 due to addition of transition (counting is 0-based)

    // Delete clip after the crossfade
    Click(VideoClip(0,3)); // Clip 3 has become index 3 due to addition of transition (counting is 0-based)
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
    Click(VideoClip(0,1));
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

    // Test moving one clip around
    wxPoint from = Center(VideoClip(0,3));
    wxPoint to = from;
    to.x = 2; // Move to the beginning of timeline

    pts length = VideoClip(0,3)->getLength();

    MouseMove(TimelinePosition() + Center(VideoClip(0,3)));
    MouseDown();
    FixtureGui::waitForIdle();

    for (int i = 1; i < 10; ++i)
    {
        wxPoint p(from.x + (from.x - to.x) / i, from.y + (from.y - to.y) / i); 
        MouseMove(TimelinePosition() + p);
        FixtureGui::waitForIdle();
    }
    MouseUp();
    FixtureGui::waitForIdle();

    ASSERT( VideoClip(0,0)->getLength() == length );
    FixtureGui::triggerUndo();
    ASSERT( VideoClip(0,3)->getLength() == length );

    // Zoom in
    Char('=');
    FixtureGui::waitForIdle();

    // Make transition after clip 2
    TrimLeft(VideoClip(0,2),30,true);
    TrimRight(VideoClip(0,1),30,true);
    Char('c');
    FixtureGui::waitForIdle();
    ASSERT(VideoClip(0,2)->isA<model::Transition>());

    // Move clip 2: the transition must be removed
    FixtureGui::getTimeline().getSelection().unselectAll();
    from = Center(VideoClip(0,1));
    to = Center(VideoClip(0,4));
    MouseMove(TimelinePosition() + from);
    MouseDown();
    for (int i = 1; i < 25; ++i)
    {
        from.x += 5;
        wxPoint p(from.x + (from.x - to.x) / i, from.y + (from.y - to.y) / i); 
        MouseMove(TimelinePosition() + p);
        FixtureGui::waitForIdle();
    }

    MouseUp();
    FixtureGui::waitForIdle();
    ASSERT(VideoClip(0,1)->isA<model::EmptyClip>());
    ASSERT(!VideoClip(0,2)->isA<model::Transition>());
    FixtureGui::pause();
}

} // namespace