#include "HelperTimeline.h"

#include <boost/foreach.hpp>
#include <ostream>
#include <wx/mousestate.h>
#include <wx/uiaction.h>
#include <wx/utils.h>
#include "AudioClip.h"
#include "AudioTrack.h"
#include "AudioView.h"
#include "ClipView.h"
#include "Cursor.h"
#include "Drag.h"
#include "EmptyClip.h"
#include "HelperApplication.h"
#include "HelperTimelinesView.h"
#include "HelperWindow.h"
#include "IClip.h"
#include "ids.h"
#include "Layout.h"
#include "Logging.h"
#include "MousePointer.h"
#include "PositionInfo.h"
#include "Scrolling.h"
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

pixel VCenter(model::TrackPtr track)
{
    pixel top = getTimeline().getSequenceView().getPosition(track);
    return top + track->getHeight() / 2;
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

wxPoint VTopQuarterLeft(model::IClipPtr clip)
{
    return wxPoint( LeftPixel(clip), VTopQuarter(clip) );
}

wxPoint VTopQuarterRight(model::IClipPtr clip)
{
    return wxPoint( RightPixel(clip), VTopQuarter(clip) );
}

wxPoint VBottomQuarterHCenter(model::IClipPtr clip)
{
    return wxPoint( HCenter(clip), VBottomQuarter(clip) );
}

wxPoint LeftVBottomQuarter(model::IClipPtr clip)
{
    // Not allowed for transitions:
    // When the clip in front of the transition has length 0, then using this position
    // on the transition causes the mouse pointer information to actually return the clip
    // in front of the transition (so that it can be selected for trimming).
    //
    // The algoritm in the method 'LeftPixel' contains code for 'adjusting the found point'
    // until the actual given clip is found (to accommodate for rounding errors). However,
    // that algorithm, in combination with the 'return the empty clip in front of the transition
    // and not the transition' causes the wrong position to be returned here.
    //
    // To resolve this issue: call this method with the clip before the transition as input.
    ASSERT(!clip->isA<model::Transition>());
    return wxPoint( LeftPixel(clip), VBottomQuarter(clip) );
}

wxPoint RightVBottomQuarter(model::IClipPtr clip)
{
    // Not allowed for transitions:
    // When the clip in front of the transition has length 0, then using this position
    // on the transition causes the mouse pointer information to actually return the clip
    // in front of the transition (so that it can be selected for trimming).
    //
    // The algoritm in the method 'RightPixel' contains code for 'adjusting the found point'
    // until the actual given clip is found (to accommodate for rounding errors). However,
    // that algorithm, in combination with the 'return the empty clip in front of the transition
    // and not the transition' causes the wrong position to be returned here.
    //
    // To resolve this issue: call this method with the clip after the transition as input.
    ASSERT(!clip->isA<model::Transition>());
    return wxPoint( RightPixel(clip), VBottomQuarter(clip) );
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

wxPoint OnTimescaleAbove(wxPoint position)
{
    wxPoint result(position);
    result.y = gui::Layout::TimeScaleHeight - 5;
    return result;
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

pixel CursorPosition()
{
    return getTimeline().getCursor().getPosition();
}

void PositionCursor(pixel position)
{
    VAR_DEBUG(position);
    Move(wxPoint(position, gui::Layout::VideoPosition - 4));
    wxUIActionSimulator().MouseClick();
}

void Move(wxPoint position)
{
    MoveWithinWidget(position,TimelinePosition() - getTimeline().getScrolling().getOffset());
    ASSERT_EQUALS(getTimeline().getMousePointer().getPosition(), position);
}

void Click(wxPoint position)
{
    Move(position);
    VAR_DEBUG(position);
    wxUIActionSimulator().MouseClick();
    waitForIdle();
    ASSERT_EQUALS(getTimeline().getMousePointer().getLeftDownPosition(), position);
}

void TrimLeft(model::IClipPtr clip, pixel length, bool shift, bool endtrim)
{
    VAR_DEBUG(clip)(length)(shift);
    wxPoint from = LeftCenter(clip);
    wxPoint to = from;
    to.x += length;
    BeginTrim(from,shift);
    Move(to);
    waitForIdle();
    if (endtrim) { EndTrim(shift); }
    waitForIdle();
}

void TrimRight(model::IClipPtr clip, pixel length, bool shift, bool endtrim)
{
    VAR_DEBUG(clip)(length)(shift);
    wxPoint from = RightCenter(clip);
    wxPoint to = from;
    to.x += length;
    BeginTrim(from,shift);
    Move(to);
    if (endtrim) { EndTrim(shift); }
}

void BeginTrim(wxPoint from, bool shift)
{
    Move(from);
    if (shift) { ShiftDown(); }
    LeftDown();
}

void EndTrim(bool shift)
{
    LeftUp();
    if (shift) { ShiftUp(); }
}

Zoom::Zoom(int level)
    : mLevel(level)
{
    for (int i = 0; i < mLevel; ++i)
    {
        Type('=');
    }
}

Zoom::~Zoom()
{
    for (int i = 0; i < mLevel; ++i)
    {
        Type('-');
    }
}

void Trim(wxPoint from, wxPoint to, bool shift)
{
    Move(from);
    LeftDown();
    if (shift) { ShiftDown(); }
    Move(to);
    LeftUp();
    if (shift) { ShiftUp(); }
}

void ShiftTrim(wxPoint from, wxPoint to)
{
    Trim(from,to,true);
}

void ToggleInterval(pixel from, pixel to)
{
    // The mouse is moved this amount of pixels before the shift is pressed in 'ShiftDrag'.
    // The interval creation does not start before shift is pressed.
    // Therefore, the initial drag is adjusted by this amount, to ensure that the position
    // where the new interval is started is known.
    pixel beforeShift = gui::Layout::DragThreshold + 1;
    if (to > from)
    {
        // In case of moving from left to right, then the 'move before shift' must be on the left side
        beforeShift = -1 * beforeShift;
    }
    static const pixel y = gui::Layout::TimeScaleHeight - 5;

    wxPoint fromPoint(from + beforeShift, y);
    wxPoint toPoint(to,y);
    wxPoint betweenPoint(fromPoint);
    betweenPoint.x += (fromPoint.x > toPoint.x) ? -(gui::Layout::DragThreshold+1) : (gui::Layout::DragThreshold+1); // Should be greater than the tolerance in StateLeftDown (otherwise, the Drag won't be started)
    Move(fromPoint);
    LeftDown();
    Move(betweenPoint);
    ShiftDown();
    Move(toPoint);
    LeftUp();
    ShiftUp();
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
    Type(' ');
    pause(ms);
    Type(' ');
    waitForIdle();
}

gui::timeline::MouseOnClipPosition LogicalPosition(wxPoint position)
{
    return getTimeline().getMousepointer().getInfo(position).logicalclipposition;
}

void DeselectAllClips()
{
    LOG_DEBUG;
    getTimeline().getSelection().unselectAll();
};

void DeleteClip(model::IClipPtr clip)
{
    Click(Center(clip));
    Type(WXK_DELETE);
}

void DumpSequenceAndWait()
{
    LOG_ERROR << DUMP(getSequence());
    Log::flush();
    pause(999999999);
}

} // namespace