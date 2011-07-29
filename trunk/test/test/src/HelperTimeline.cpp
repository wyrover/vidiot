#include "HelperTimeline.h"

#include <wx/uiaction.h>
#include <boost/foreach.hpp>
#include "ClipView.h"
#include "EmptyClip.h"
#include "HelperApplication.h"
#include "HelperTimelinesView.h"
#include "IClip.h"
#include "Selection.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "Timeline.h"
#include "Track.h"
#include "ViewMap.h"

namespace test {

wxPoint TimelinePosition()
{
    return getTimeline().GetScreenPosition();
}

int NumberOfVideoClipsInTrack(int trackindex)
{
    model::TrackPtr videoTrack = getSequence()->getVideoTrack(trackindex);
    return videoTrack->getClips().size();
}

model::IClipPtr VideoClip(int trackindex, int clipindex)
{
    model::TrackPtr videoTrack = getSequence()->getVideoTrack(trackindex);
    return videoTrack->getClipByIndex(clipindex);
}

int getNonEmptyClipsCount()
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

int getSelectedClipsCount()
{
    int result = getTimeline().getSelection().getClips().size();
    VAR_DEBUG(result);
    return result;
}

pixel LeftPixel(model::IClipPtr clip)
{
    return getTimeline().getViewMap().getView(clip)->getLeftPixel();
}

pixel RightPixel(model::IClipPtr clip)
{
    return getTimeline().getViewMap().getView(clip)->getRightPixel();
}

pixel TopPixel(model::IClipPtr clip)
{
    return getTimeline().getSequenceView().getPosition(clip->getTrack());
}

pixel BottomPixel(model::IClipPtr clip)
{
    return TopPixel(clip) + clip->getTrack()->getHeight();
}

wxPoint Center(model::IClipPtr clip)
{
    return wxPoint( (LeftPixel(clip) + RightPixel(clip)) / 2, (TopPixel(clip) + BottomPixel(clip)) / 2);
}

wxPoint RightCenter(model::IClipPtr clip)
{
    return wxPoint( RightPixel(clip), (TopPixel(clip) + BottomPixel(clip)) / 2);
}

wxPoint LeftCenter(model::IClipPtr clip)
{
    return wxPoint( LeftPixel(clip), (TopPixel(clip) + BottomPixel(clip)) / 2);
}

void Click(model::IClipPtr clip)
{
    // yposition
    pixel trackY = getTimeline().getSequenceView().getPosition(clip->getTrack());
    pixel trackH = clip->getTrack()->getHeight();
    pixel clickY = trackY + (trackH / 2);

    // xposition
    pixel clickX = (LeftPixel(clip) + RightPixel(clip)) / 2;

    wxUIActionSimulator().MouseMove(getTimeline().GetScreenPosition() + wxPoint(clickX, clickY));
    wxUIActionSimulator().MouseClick();
    waitForIdle();
}

void TrimLeft(model::IClipPtr clip, pixel length, bool shift)
{
    wxPoint from = LeftCenter(clip);
    from.x += 1; // The +1 is required to fix errors where the pointer is moved to a slightly different position (don't know why exactly)
    wxPoint to = from;
    to.x += length;
    wxUIActionSimulator().MouseMove(TimelinePosition() + from);
    if (shift) wxUIActionSimulator().KeyDown(0, wxMOD_SHIFT);
    wxUIActionSimulator().MouseDown();
    wxUIActionSimulator().MouseMove(TimelinePosition() + to);
    wxUIActionSimulator().MouseUp();
    if (shift) wxUIActionSimulator().KeyUp(0, wxMOD_SHIFT);
    waitForIdle();
}

void TrimRight(model::IClipPtr clip, pixel length, bool shift)
{
    wxPoint from = RightCenter(clip);
    from.x -= 1; // The -1 is required to fix errors where the pointer is moved to a slightly different position (don't know why exactly)
    wxPoint to = from;
    to.x -= length;
    wxUIActionSimulator().MouseMove(TimelinePosition() + from);
    if (shift) wxUIActionSimulator().KeyDown(0, wxMOD_SHIFT);
    wxUIActionSimulator().MouseDown();
    wxUIActionSimulator().MouseMove(TimelinePosition() + to);
    wxUIActionSimulator().MouseUp();
    if (shift) wxUIActionSimulator().KeyUp(0, wxMOD_SHIFT);
    waitForIdle();
}

void Drag(wxPoint from, wxPoint to)
{
    wxUIActionSimulator().MouseMove(TimelinePosition() + from);
    wxUIActionSimulator().MouseDown();
    waitForIdle();
    for (int i = 10; i > 0; --i)
    {
        wxPoint p(from.x + (to.x - from.x) / i, from.y + (to.y - from.y) / i); 
        wxUIActionSimulator().MouseMove(TimelinePosition() + p);
        waitForIdle();
    }
    waitForIdle();
    wxUIActionSimulator().MouseUp();
    waitForIdle();
}

void ASSERT_SELECTION_SIZE(int size)
{
    ASSERT(getSelectedClipsCount() == 2 * size); // * 2 since AudioClips are selected also
}

} // namespace