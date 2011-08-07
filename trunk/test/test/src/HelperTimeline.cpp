#include "HelperTimeline.h"

#include <wx/uiaction.h>
#include <wx/mousestate.h>
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
#include "MousePointer.h"
#include "PositionInfo.h"
#include "Selection.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "Timeline.h"
#include "Track.h"
#include "Transition.h"
#include "UtilLogWxwidgets.h"
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

model::VideoTransitionPtr VideoTransition(int trackindex, int clipindex)
{
    model::VideoTransitionPtr t = boost::dynamic_pointer_cast<model::VideoTransition>(getSequence()->getVideoTrack(trackindex)->getClipByIndex(clipindex));
    ASSERT(t);
    return t;
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
    wxPoint p( getTimeline().getViewMap().getView(clip)->getLeftPixel(), VCenter(clip) );
    gui::timeline::PointerPositionInfo info =  getTimeline().getMousepointer().getInfo(p);
    while (info.clip != clip)
    {
        // This special handling is required to adjust for rounding errors in case of zooming.
        // Given that 'getInfo' first looks upon the leftmost clip, and the fact that the right
        // edge of that clip may overlap with the left edge of the given clip, we need to look
        // more to the right to find the left most pixel of this clip that will be found in
        // mouse lookup functions (getInfo()).
        p.x++;
        info = getTimeline().getMousepointer().getInfo(p);
    }
    ASSERT(info.clip == clip);
    return p.x;
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

pixel VCenter(model::IClipPtr clip)
{
    return (TopPixel(clip) + BottomPixel(clip)) / 2;
}

pixel HCenter(model::IClipPtr clip)
{
    return (LeftPixel(clip) + RightPixel(clip)) / 2;
}

wxPoint Center(model::IClipPtr clip)
{
    return wxPoint( HCenter(clip), VCenter(clip) );
}

wxPoint RightCenter(model::IClipPtr clip)
{
    return wxPoint( RightPixel(clip), VCenter(clip) );
}

wxPoint LeftCenter(model::IClipPtr clip)
{
    return wxPoint( LeftPixel(clip), VCenter(clip) );
}

void PositionCursor(pixel position)
{
    VAR_DEBUG(position);
    Move(wxPoint(position, gui::Layout::sVideoPosition - 4));
    wxUIActionSimulator().MouseClick();
}

void Move(wxPoint position)
{
    VAR_DEBUG(position);
    wxPoint absoluteposition = getTimeline().GetScreenPosition() + position;
    wxUIActionSimulator().MouseMove(absoluteposition);
    ASSERT_EQUALS(wxGetMouseState().GetPosition(), absoluteposition);
}

void Click(wxPoint position)
{
    Move(position);
    VAR_DEBUG(position);
    wxUIActionSimulator().MouseClick();
    waitForIdle();
    ASSERT_EQUALS(getTimeline().getMousePointer().getLeftDownPosition(), position);
}

void TrimLeft(model::IClipPtr clip, pixel length, bool shift)
{
    VAR_DEBUG(clip)(length)(shift);
    wxPoint from = LeftCenter(clip);
    wxPoint to = from;
    to.x += length;
    Move(from);
    ASSERT(getTimeline().getMousepointer().getInfo(from).logicalclipposition == gui::timeline::ClipBegin)(getTimeline().getMousepointer().getInfo(from));
    ASSERT(getTimeline().getMousepointer().getInfo(from).clip == clip)(getTimeline().getMousepointer().getInfo(from));
    if (shift) wxUIActionSimulator().KeyDown(0, wxMOD_SHIFT);
    wxUIActionSimulator().MouseDown();
    waitForIdle();
    Move(to);
    waitForIdle();
    wxUIActionSimulator().MouseUp();
    if (shift) wxUIActionSimulator().KeyUp(0, wxMOD_SHIFT);
    waitForIdle();
}

void TrimRight(model::IClipPtr clip, pixel length, bool shift)
{
    VAR_DEBUG(clip)(length)(shift);
    wxPoint from = RightCenter(clip);
    wxPoint to = from;
    to.x -= length;
    Move(from);
    if (shift) wxUIActionSimulator().KeyDown(0, wxMOD_SHIFT);
    wxUIActionSimulator().MouseDown();
    Move(to);
    wxUIActionSimulator().MouseUp();
    if (shift) wxUIActionSimulator().KeyUp(0, wxMOD_SHIFT);
    waitForIdle();
}

void Drag(wxPoint from, wxPoint to, bool ctrl)
{
    VAR_DEBUG(from)(to)(ctrl);
    if (ctrl) { ControlDown(); }
    waitForIdle();
    Move(from);
    waitForIdle();
    wxUIActionSimulator().MouseDown();
    waitForIdle();
    if (ctrl) { ControlUp(); }
    waitForIdle();
    for (int i = 10; i > 0; --i)
    {
        wxPoint p(from.x + (to.x - from.x) / i, from.y + (to.y - from.y) / i); 
        Move(p);
        pause(100);
        waitForIdle();
    }
    waitForIdle();
    wxUIActionSimulator().MouseUp();
    waitForIdle();
}

gui::timeline::MouseOnClipPosition LogicalPosition(wxPoint position)
{
    return getTimeline().getMousepointer().getInfo(position).logicalclipposition;
}

void ASSERT_SELECTION_SIZE(int size)
{
    ASSERT_EQUALS(getSelectedClipsCount(),2 * size); // * 2 since AudioClips are selected also
}

void DeselectAllClips()
{
    LOG_DEBUG;
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