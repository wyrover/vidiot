#include "HelperTimeline.h"

#include <wx/uiaction.h>
#include <boost/foreach.hpp>
#include "AudioClip.h"
#include "AudioTrack.h"
#include "ClipView.h"
#include "EmptyClip.h"
#include "HelperApplication.h"
#include "HelperTimelinesView.h"
#include "HelperWindow.h"
#include "IClip.h"
#include "Layout.h"
#include "Selection.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "Timeline.h"
#include "Track.h"
#include "Transition.h"
#include "VideoClip.h"
#include "VideoTrack.h"
#include "VideoTransition.h"
#include "ViewMap.h"
#include "Zoom.h"

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

model::VideoTrackPtr VideoTrack(int trackindex)
{
    return boost::dynamic_pointer_cast<model::VideoTrack>(getSequence()->getVideoTrack(trackindex));
}

model::AudioTrackPtr AudioTrack(int trackindex)
{
    return boost::dynamic_pointer_cast<model::AudioTrack>(getSequence()->getAudioTrack(trackindex));
}

model::IClipPtr VideoClip(int trackindex, int clipindex)
{
    return getSequence()->getVideoTrack(trackindex)->getClipByIndex(clipindex);
}

model::IClipPtr AudioClip(int trackindex, int clipindex)
{
    return getSequence()->getAudioTrack(trackindex)->getClipByIndex(clipindex);
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

void PositionCursor(pixel position)
{
    wxUIActionSimulator().MouseMove(getTimeline().GetScreenPosition() + wxPoint(gui::Layout::sVideoPosition - 4, position));
    wxUIActionSimulator().MouseClick();
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

void Drag(wxPoint from, wxPoint to, bool ctrl)
{
    if (ctrl) { ControlDown(); }
    wxUIActionSimulator().MouseMove(TimelinePosition() + from);
    wxUIActionSimulator().MouseDown();
    if (ctrl) { ControlUp(); }
    waitForIdle();
    for (int i = 10; i > 0; --i)
    {
        wxPoint p(from.x + (to.x - from.x) / i, from.y + (to.y - from.y) / i); 
        wxUIActionSimulator().MouseMove(TimelinePosition() + p);
        //pause(100);
        waitForIdle();
    }
    waitForIdle();
    wxUIActionSimulator().MouseUp();
    waitForIdle();
}

void ASSERT_SELECTION_SIZE(int size)
{
    ASSERT_EQUALS(getSelectedClipsCount(),2 * size); // * 2 since AudioClips are selected also
}

void DeselectAllClips()
{
    getTimeline().getSelection().unselectAll();
};

void DumpSequence()
{
    model::SequencePtr sequence = getSequence();
    wxString tab("    ");
    LOG_DEBUG << "============================================================";
    VAR_DEBUG(*sequence);
    int tracknum = 0;
    BOOST_FOREACH( model::TrackPtr track, sequence->getVideoTracks() )
    {
        LOG_DEBUG << "-------------------- VIDEOTRACK " << tracknum++ << " (length=" << track->getLength() << ") --------------------";
        LOG_DEBUG << tab << "TRACK " << *track;
        BOOST_FOREACH( model::IClipPtr clip, track->getClips() )
        {
            if (clip->isA<model::VideoTransition>())
            {
                LOG_DEBUG << tab << tab << "TRANSITION " << *(boost::dynamic_pointer_cast<model::VideoTransition>(clip));
            }
            else if (clip->isA<model::EmptyClip>())
            {
                LOG_DEBUG << tab << tab << "EMPTY      " << *(boost::dynamic_pointer_cast<model::EmptyClip>(clip));
            }
            else
            {
                LOG_DEBUG << tab << tab << "CLIP       " << *(boost::dynamic_pointer_cast<model::VideoClip>(clip));
            }
        }
    }
    tracknum = 0;
    BOOST_FOREACH( model::TrackPtr track, sequence->getAudioTracks() )
    {
        LOG_DEBUG << "-------------------- AUDIOTRACK " << tracknum++ << " (length=" << track->getLength() << ") --------------------";
        LOG_DEBUG << tab << *track;
        BOOST_FOREACH( model::IClipPtr clip, track->getClips() )
        {
            //if (clip->isA<model::AudioTransition>())
            //{
            //    LOG_DEBUG << tab << tab << "TRANSITION " << *(boost::dynamic_pointer_cast<model::AudioTransition>(clip));
            //}
            //else
            if (clip->isA<model::EmptyClip>())
            {
                LOG_DEBUG << tab << tab << "EMPTY      " << *(boost::dynamic_pointer_cast<model::EmptyClip>(clip));
            }
            else
            {
                LOG_DEBUG << tab << tab << "CLIP       " << *(boost::dynamic_pointer_cast<model::AudioClip>(clip));
            }
        }
    }
    LOG_DEBUG << "============================================================";
}

} // namespace