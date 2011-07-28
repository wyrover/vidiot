#include "HelperTimeline.h"

#include <boost/foreach.hpp>
#include "AutoFolder.h"
#include "ClipView.h"
#include "EmptyClip.h"
#include "FixtureGui.h"
#include "IClip.h"
#include "Selection.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "Timeline.h"
#include "Track.h"
#include "UtilLog.h"
#include "ViewMap.h"
#include "Window.h"

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

HelperTimeline::HelperTimeline()
{
}

HelperTimeline::~HelperTimeline()
{
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

wxPoint HelperTimeline::TimelinePosition()
{
    return getTimeline().GetScreenPosition();
}

int HelperTimeline::NumberOfVideoClipsInTrack(int trackindex)
{
    model::TrackPtr videoTrack = getSequence()->getVideoTrack(trackindex);
    return videoTrack->getClips().size();
}

model::IClipPtr HelperTimeline::VideoClip(int trackindex, int clipindex)
{
    model::TrackPtr videoTrack = getSequence()->getVideoTrack(trackindex);
    return videoTrack->getClipByIndex(clipindex);
}

int HelperTimeline::getNonEmptyClipsCount()
{
    int result = 0;
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getTracks() )
    {
        BOOST_FOREACH( model::IClipPtr clip, track->getClips() )
        {
            if (!clip->isA<model::EmptyClip>())
            {
                result++;
            }
        }
    }
    return result;
}

int HelperTimeline::getSelectedClipsCount()
{
    int result = getTimeline().getSelection().getClips().size();
    VAR_DEBUG(result);
    return result;
}

pixel HelperTimeline::LeftPixel(model::IClipPtr clip)
{
    return getTimeline().getViewMap().getView(clip)->getLeftPixel();
}

pixel HelperTimeline::RightPixel(model::IClipPtr clip)
{
    return getTimeline().getViewMap().getView(clip)->getRightPixel();
}

pixel HelperTimeline::TopPixel(model::IClipPtr clip)
{
    return getTimeline().getSequenceView().getPosition(clip->getTrack());
}

pixel HelperTimeline::BottomPixel(model::IClipPtr clip)
{
    return TopPixel(clip) + clip->getTrack()->getHeight();
}

wxPoint HelperTimeline::Center(model::IClipPtr clip)
{
    return wxPoint( (LeftPixel(clip) + RightPixel(clip)) / 2, (TopPixel(clip) + BottomPixel(clip)) / 2);
}

wxPoint HelperTimeline::RightCenter(model::IClipPtr clip)
{
    return wxPoint( RightPixel(clip), (TopPixel(clip) + BottomPixel(clip)) / 2);
}

wxPoint HelperTimeline::LeftCenter(model::IClipPtr clip)
{
    return wxPoint( LeftPixel(clip), (TopPixel(clip) + BottomPixel(clip)) / 2);
}

void HelperTimeline::Click(model::IClipPtr clip)
{
    // yposition
    pixel trackY = getTimeline().getSequenceView().getPosition(clip->getTrack());
    pixel trackH = clip->getTrack()->getHeight();
    pixel clickY = trackY + (trackH / 2);

    // xposition
    pixel clickX = (LeftPixel(clip) + RightPixel(clip)) / 2;

    MouseMove(getTimeline().GetScreenPosition() + wxPoint(clickX, clickY));
    MouseClick();
    waitForIdle();
}

void HelperTimeline::TrimLeft(model::IClipPtr clip, pixel length, bool shift)
{
    wxPoint from = LeftCenter(clip);
    from.x += 1; // The +1 is required to fix errors where the pointer is moved to a slightly different position (don't know why exactly)
    wxPoint to = from;
    to.x += length;
    MouseMove(TimelinePosition() + from);
    if (shift) KeyDown(0, wxMOD_SHIFT);
    MouseDown();
    MouseMove(TimelinePosition() + to);
    MouseUp();
    if (shift) KeyUp(0, wxMOD_SHIFT);
    waitForIdle();
}

void HelperTimeline::TrimRight(model::IClipPtr clip, pixel length, bool shift)
{
    wxPoint from = RightCenter(clip);
    from.x -= 1; // The -1 is required to fix errors where the pointer is moved to a slightly different position (don't know why exactly)
    wxPoint to = from;
    to.x -= length;
    MouseMove(TimelinePosition() + from);
    if (shift) KeyDown(0, wxMOD_SHIFT);
    MouseDown();
    MouseMove(TimelinePosition() + to);
    MouseUp();
    if (shift) KeyUp(0, wxMOD_SHIFT);
    waitForIdle();
}

void HelperTimeline::Drag(wxPoint from, wxPoint to)
{
    MouseMove(TimelinePosition() + from);
    MouseDown();
    waitForIdle();
    for (int i = 10; i > 0; --i)
    {
        wxPoint p(from.x + (to.x - from.x) / i, from.y + (to.y - from.y) / i); 
        MouseMove(TimelinePosition() + p);
        waitForIdle();
    }
    waitForIdle();
    MouseUp();
    waitForIdle();
}

void HelperTimeline::ASSERT_SELECTION_SIZE(int size)
{
    ASSERT(getSelectedClipsCount() == 2 * size); // * 2 since AudioClips are selected also
}

} // namespace