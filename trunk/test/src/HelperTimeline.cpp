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

namespace test {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

HelperTimeline::HelperTimeline()
    :   TestFilesPath("D:\\Vidiot\\test", "")
    ,   InputFiles(model::AutoFolder::getSupportedFiles(TestFilesPath))
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
    return FixtureGui::getTimeline().GetScreenPosition();
}

int HelperTimeline::NumberOfVideoClipsInTrack(int trackindex)
{
    model::TrackPtr videoTrack = FixtureGui::getActiveSequence()->getVideoTrack(trackindex);
    return videoTrack->getClips().size();
}

model::IClipPtr HelperTimeline::VideoClip(int trackindex, int clipindex)
{
    model::TrackPtr videoTrack = FixtureGui::getActiveSequence()->getVideoTrack(trackindex);
    return videoTrack->getClipByIndex(clipindex);
}

int HelperTimeline::getNonEmptyClipsCount()
{
    int result = 0;
    BOOST_FOREACH( model::TrackPtr track, FixtureGui::getActiveSequence()->getTracks() )
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
    int result = FixtureGui::getTimeline().getSelection().getClips().size();
    VAR_DEBUG(result);
    return result;
}

pixel HelperTimeline::LeftPixel(model::IClipPtr clip)
{
    return FixtureGui::getTimeline().getViewMap().getView(clip)->getLeftPixel();
}

pixel HelperTimeline::RightPixel(model::IClipPtr clip)
{
    return FixtureGui::getTimeline().getViewMap().getView(clip)->getRightPixel();
}

pixel HelperTimeline::TopPixel(model::IClipPtr clip)
{
    return FixtureGui::getTimeline().getSequenceView().getPosition(clip->getTrack());
}

pixel HelperTimeline::BottomPixel(model::IClipPtr clip)
{
    return TopPixel(clip) + clip->getTrack()->getHeight();
}

void HelperTimeline::click(model::IClipPtr clip)
{
    // yposition
    pixel trackY = FixtureGui::getTimeline().getSequenceView().getPosition(clip->getTrack());
    pixel trackH = clip->getTrack()->getHeight();
    pixel clickY = trackY + (trackH / 2);

    // xposition
    pixel clickX = (LeftPixel(clip) + RightPixel(clip)) / 2;

    MouseMove(FixtureGui::getTimeline().GetScreenPosition() + wxPoint(clickX, clickY));
    MouseClick();
    FixtureGui::waitForIdle();
}

void HelperTimeline::ASSERT_SELECTION_SIZE(int size)
{
    ASSERT(getSelectedClipsCount() == 2 * size); // * 2 since AudioClips are selected also
}

} // namespace