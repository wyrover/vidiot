#include "HelperTimeline.h"

#include <boost/foreach.hpp>
#include <wx/mousestate.h>
#include <wx/uiaction.h>
#include "AudioClip.h"
#include "AudioTrack.h"
#include "AudioView.h"
#include "ClipView.h"
#include "Drag.h"
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
#include "VideoView.h"
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

int NumberOfAudioClipsInTrack(int trackindex)
{
    model::TrackPtr audioTrack = getSequence()->getAudioTrack(trackindex);
    return audioTrack->getClips().size();
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
    int index = clipindex;
    if (clipindex < 0)
    {
        index = NumberOfVideoClipsInTrack(trackindex);
        index += clipindex;
    }
    return getSequence()->getVideoTrack(trackindex)->getClipByIndex(index);
}

model::VideoTransitionPtr VideoTransition(int trackindex, int clipindex)
{
    model::VideoTransitionPtr t = boost::dynamic_pointer_cast<model::VideoTransition>(getSequence()->getVideoTrack(trackindex)->getClipByIndex(clipindex));
    ASSERT(t);
    return t;
}

model::IClipPtr AudioClip(int trackindex, int clipindex)
{
    int index = clipindex;
    if (clipindex < 0)
    {
        index = NumberOfAudioClipsInTrack(trackindex);
        index += clipindex;
    }
    return getSequence()->getAudioTrack(trackindex)->getClipByIndex(index);
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
    int result = getSequence()->getSelectedClips().size();
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
        // The leftmost pts value may not be corresponding with an exact pixel value.
        // Thus sometimes, we need to look more to the right to find the leftmost pixel of
        // this clip - to ensure that lookups at the returned pixel value correspond with the
        // given clip and not its left neighbour.
        p.x++;
        info = getTimeline().getMousepointer().getInfo(p);
    }
    ASSERT(info.clip == clip);
    return p.x;
}

pixel RightPixel(model::IClipPtr clip)
{
    wxPoint p( getTimeline().getViewMap().getView(clip)->getRightPixel(), VCenter(clip) );
    gui::timeline::PointerPositionInfo info =  getTimeline().getMousepointer().getInfo(p);
    while (info.clip != clip)
    {
        // This special handling is required to adjust for rounding errors in case of zooming.
        // The rightmost pts value may not be corresponding with an exact pixel value.
        // Thus sometimes, we need to look more to the left to find the rightmost pixel of
        // this clip - to ensure that lookups at the returned pixel value correspond with the
        // given clip and not its right neighbour.
        p.x--;
        info = getTimeline().getMousepointer().getInfo(p);
    }
    ASSERT(info.clip == clip);
    return p.x;
}

pixel TopPixel(model::IClipPtr clip)
{
    return getTimeline().getSequenceView().getPosition(clip->getTrack());
}

pixel BottomPixel(model::IClipPtr clip)
{
    return TopPixel(clip) + clip->getTrack()->getHeight() - 1;
}

pixel VCenter(model::IClipPtr clip)
{
    return (TopPixel(clip) + BottomPixel(clip)) / 2;
}

pixel VTopQuarter(model::IClipPtr clip)
{
    return TopPixel(clip) + clip->getTrack()->getHeight() / 4;
}

pixel VBottomQuarter(model::IClipPtr clip)
{
    return BottomPixel(clip) - clip->getTrack()->getHeight() / 4;
}

pixel HCenter(model::IClipPtr clip)
{
    return (LeftPixel(clip) + RightPixel(clip)) / 2;
}

wxPoint Center(model::IClipPtr clip)
{
    return wxPoint( HCenter(clip), VCenter(clip) );
}

wxPoint VTopQuarterHCenter(model::IClipPtr clip)
{
    return wxPoint( HCenter(clip), VTopQuarter(clip) );
}

wxPoint VBottomQuarterHCenter(model::IClipPtr clip)
{
    return wxPoint( HCenter(clip), VBottomQuarter(clip) );
}

wxPoint LeftCenter(model::IClipPtr clip)
{
    return wxPoint( LeftPixel(clip), VCenter(clip) );
}

wxPoint RightCenter(model::IClipPtr clip)
{
    return wxPoint( RightPixel(clip), VCenter(clip) );
}

wxPoint LeftBottom(model::IClipPtr clip)
{
    return wxPoint(LeftPixel(clip),BottomPixel(clip));
}

wxPoint RightBottom(model::IClipPtr clip)
{
    return wxPoint(RightPixel(clip),BottomPixel(clip));
}

pixel LeftPixel(DraggedClips drag)
{
    return getTimeline().getDrag().getBitmapPosition().x;
}

pixel RightPixel(DraggedClips drag)
{
    return getTimeline().getDrag().getBitmapPosition().x + getTimeline().getDrag().getBitmapSize().x;
}

pixel TopPixel(DraggedClips drag)
{
    return getTimeline().getDrag().getBitmapPosition().y;
}

pixel BottomPixel(DraggedClips drag)
{
    return getTimeline().getDrag().getBitmapPosition().y + getTimeline().getDrag().getBitmapSize().y;
}

pixel VCenter(DraggedClips drag)
{
    return (TopPixel(drag) + BottomPixel(drag)) / 2;
}

pixel HCenter(DraggedClips drag)
{
    return (LeftPixel(drag) + RightPixel(drag)) / 2;
}

wxPoint Center(DraggedClips drag)
{
    return wxPoint( HCenter(drag), VCenter(drag) );
}

wxPoint LeftCenter(DraggedClips drag)
{
    return wxPoint( LeftPixel(drag), VCenter(drag) );
}

wxPoint RightCenter(DraggedClips drag)
{
    return wxPoint( RightPixel(drag), VCenter(drag) );
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
    int count = 0;
	while (wxGetMouseState().GetPosition() != absoluteposition)
	{
		// Loop is required since sometimes the move fails the first time.
		// Particularly seen when working through remote desktop/using touchpad.
		wxUIActionSimulator().MouseMove(absoluteposition);
		waitForIdle();
        if (++count > 3) break;
	}
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

void Drag(wxPoint from, wxPoint to, bool ctrl, bool mousedown, bool mouseup)
{
    VAR_DEBUG(from)(to)(ctrl);
    if (ctrl)
    {
        ControlDown();
    }
    Move(from);
    if (mousedown)
    {
        wxUIActionSimulator().MouseDown();
        waitForIdle();
    }
    if (ctrl)
    {
        ControlUp();
        waitForIdle();
    }
    static const int DRAGSTEPS = 10; // Use a higher number to see the drag in small steps. NOTE: Too small number causes drop in wrong position!
    for (int i = DRAGSTEPS; i > 0; --i)
    {
        wxPoint p(from.x + (to.x - from.x) / i, from.y + (to.y - from.y) / i);
        Move(p);
    }
    if (mouseup)
    {
        wxUIActionSimulator().MouseUp();
        waitForIdle();
    }
}

void ShiftDrag(wxPoint from, wxPoint to)
{
    wxPoint between(from);
    between.x += (from.x > to.x) ? -3 : 3; // Should be greater than the tolerance in StateLeftDown (otherwise, the Drag won't be started)
    Drag(from, between, false, true, false);
    ShiftDown();
    Move(to);
    waitForIdle();
    wxUIActionSimulator().MouseUp();
    ShiftUp();
    waitForIdle();
}

void DragAlign(wxPoint from, pixel position, bool shift, bool left)
{
    ASSERT_DIFFERS(from.x, position);
    wxPoint to(from);
    to.x += (from.x > position) ? -3 : 3; // Should be greater than the tolerance in StateLeftDown (otherwise, the Drag won't be started)
    Drag(from, to, false, true, false);
    if (shift)
    {
        ShiftDown();
    }
	// Now drag until the left position of the drag is aligned with position
	pixel alignposition = left ? LeftPixel(DraggedClips()) : RightPixel(DraggedClips());
    to.x += (position - alignposition);
	Move(to);
	wxUIActionSimulator().MouseUp();
    if (shift)
    {
        ShiftUp();
    }
    waitForIdle();
}

void DragAlignLeft(wxPoint from, pixel position)
{
	DragAlign(from,position,false,true);
}

void ShiftDragAlignLeft(wxPoint from, pixel position)
{
    DragAlign(from,position,true,true);
}

void DragAlignRight(wxPoint from, pixel position)
{
	DragAlign(from,position,false,false);
}

void ShiftDragAlignRight(wxPoint from, pixel position)
{
	DragAlign(from,position,true,false);
}

void Scrub(pixel from, pixel to)
{
    VAR_DEBUG(from)(to);
    for (int i = from; i < to; ++i)
    {
        PositionCursor(i);
    }
}

void Play(pixel from, int ms)
{
    PositionCursor(from);
    // NOTE: Don't use waitForIdle() when the video is playing!!!
    //       When the video is playing, the system does not become Idle (playback events).
    wxUIActionSimulator().Char(' ');
    pause(ms);
    wxUIActionSimulator().Char(' ');
    waitForIdle();
}

gui::timeline::MouseOnClipPosition LogicalPosition(wxPoint position)
{
    return getTimeline().getMousepointer().getInfo(position).logicalclipposition;
}

void ASSERT_NO_TRANSITIONS_IN_VIDEO_TRACK(int trackindex)
{
    for (int i = 0; i < NumberOfVideoClipsInTrack(trackindex); ++i)
    {
        ASSERT(!VideoClip(trackindex,i)->isA<model::Transition>())(i);
    }
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

void DumpTimeline()
{
    model::SequencePtr sequence = getSequence();
    wxString tab("    ");
    LOG_DEBUG << "============================================================";
    LOG_DEBUG << "LEFTDOWN:  " << getTimeline().getMousePointer().getLeftDownPosition();
    LOG_DEBUG << "RIGHTDOWN: " << getTimeline().getMousePointer().getRightDownPosition();
    LOG_DEBUG << "============================================================";
    VAR_DEBUG(*sequence);
    int tracknum = 0;
    BOOST_FOREACH( model::TrackPtr track, sequence->getVideoTracks() )
    {
        LOG_DEBUG
            << "-------------------- VIDEOTRACK " << tracknum++
            << " (length=" << track->getLength()
            << ", position=" << getTimeline().getSequenceView().getVideoPosition() + getTimeline().getSequenceView().getVideo().getPosition(track)
            << ") --------------------";
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