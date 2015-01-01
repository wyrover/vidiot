// Copyright 2013-2015 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#include "Test.h"

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
    for ( model::TrackPtr track : getSequence()->getTracks() )
    {
        for ( model::IClipPtr clip : track->getClips() )
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
    return TopPixel(track) + track->getHeight() / 2;
}

pixel TopPixel(model::TrackPtr track)
{
    return getTimeline().getViewMap().getView(track)->getY();
}

pixel RightPixel(model::TrackPtr track)
{
    int nClips = track->getClips().size();
    if (nClips > 0)
    {
        return RightPixel(track->getClipByIndex(nClips - 1));
    }
    return 0;
}

pixel LeftPixel(model::IClipPtr clip)
{
    wxPoint p( getTimeline().getViewMap().getView(clip)->getLeftPixel(), VCenter(clip) );
    gui::timeline::PointerPositionInfo info =  getTimeline().getMouse().getInfo(p);
    while (info.clip != clip || info.logicalclipposition == gui::timeline::TransitionLeftClipEnd)
    {
        // This special handling is required to adjust for rounding errors in case of zooming.
        // The leftmost pts value may not be corresponding with an exact pixel value.
        // Thus sometimes, we need to look more to the right to find the leftmost pixel of
        // this clip - to ensure that lookups at the returned pixel value correspond with the
        // given clip and not its left neighbour.
        //
        // The check for TransitionLeftClipEnd is added to ensure that - in case of a
        // cross transition - not accidentally the rightmost point of the left adjacent clip
        // (adjacent to the transition) is returned; because in that case info.clip would 
        // equal the transition, see ClipView::getPositionInfo where info.clip is adjusted
        // to become 'nextTransition'.
        p.x++;
        info = getTimeline().getMouse().getInfo(p);
    }
    ASSERT(info.clip == clip);
    return p.x;
}

pixel RightPixel(model::IClipPtr clip)
{
    wxPoint p( getTimeline().getViewMap().getView(clip)->getRightPixel(), VCenter(clip) );
    gui::timeline::PointerPositionInfo info =  getTimeline().getMouse().getInfo(p);
    while (info.clip != clip || info.logicalclipposition == gui::timeline::TransitionRightClipBegin)
    {
        // This special handling is required to adjust for rounding errors in case of zooming.
        // The rightmost pts value may not be corresponding with an exact pixel value.
        // Thus sometimes, we need to look more to the left to find the rightmost pixel of
        // this clip - to ensure that lookups at the returned pixel value correspond with the
        // given clip and not its right neighbour.
        //
        // The check for TransitionRightClipBegin is added to ensure that - in case of a
        // cross transition - not accidentally the leftmost point of the right adjacent clip
        // (adjacent to the transition) is returned; because in that case info.clip would 
        // equal the transition, see ClipView::getPositionInfo where info.clip is adjusted
        // to become 'prevTransition'.

        p.x--;
        info = getTimeline().getMouse().getInfo(p);
    }
    ASSERT(info.clip == clip);
    return p.x;
}

pixel TopPixel(model::IClipPtr clip)
{
    return TopPixel(clip->getTrack());
}

pixel BottomPixel(model::IClipPtr clip)
{
    return TopPixel(clip) + getTimeline().getViewMap().getView(clip)->getH() - 1;
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

void TimelineTriggerWheel(int nSteps)
{
    util::thread::RunInMainAndWait([nSteps]
    {
        getTimeline().getStateMachine().processWheelEvent(nSteps);
    });
}

pixel CursorPosition()
{
    return getTimeline().getScrolling().ptsToPixel(getTimeline().getCursor().getLogicalPosition());
}

void TimelinePositionCursor(pixel position)
{
    VAR_DEBUG(position);
    TimelineLeftClick(wxPoint(position, gui::Layout::VideoPosition - 4));
}

void TimelineZoomIn(int level)
{
    for (int i = 0; i < level; ++i)
    {
        TimelineKeyPress('=');
    }
}

void TimelineZoomOut(int level)
{
    for (int i = 0; i < level; ++i)
    {
        TimelineKeyPress('-');
    }
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
    TimelineMove(fromPoint);
    TimelineLeftDown();
    TimelineMove(betweenPoint);
    TimelineKeyDown(WXK_SHIFT);
    TimelineMove(toPoint);
    TimelineLeftUp();
    TimelineKeyUp(WXK_SHIFT);
}

void Scrub(pixel from, pixel to)
{
    VAR_DEBUG(from)(to);
    for (int i = from; i < to; ++i)
    {
        TimelinePositionCursor(i);
    }
}

void ScrollWithRightMouseButton(pixel distance)
{
    VAR_DEBUG(distance);
    pixel maxW = getTimeline().GetClientSize().GetWidth() - 20;
    pixel y = gui::Layout::TimeScaleHeight + 2;
    ASSERT_LESS_THAN(distance,maxW);
    TimelineMove(wxPoint(maxW - 10, y));
    TimelineRightDown();
    TimelineMove(wxPoint(maxW - 10 - distance, y));
    TimelineRightUp();
}

gui::timeline::MouseOnClipPosition LogicalPosition(wxPoint position)
{
    return getTimeline().getMouse().getInfo(position).logicalclipposition;
}

void DeselectAllClips()
{
    LOG_DEBUG;
    util::thread::RunInMainAndWait([]
    {
        getTimeline().getSelection().unselectAll();
    });
};

void DeleteClip(model::IClipPtr clip, bool shift)
{
    TimelineLeftClick(Center(clip));
    if (shift)
    {
        TimelineKeyDown(WXK_SHIFT);
        TimelineKeyPress(WXK_DELETE);
        TimelineKeyUp(WXK_SHIFT);
    }
    else
    {
        TimelineKeyPress(WXK_DELETE);
    }
}

void ShiftDeleteClip(model::IClipPtr clip)
{
    DeleteClip(clip,true);
}

void DumpSequence()
{
    LOG_ERROR << DUMP(getSequence());
    Log::flush();
}

void DumpSequenceAndWait()
{
    DumpSequence();
    pause(999999999);
}

//////////////////////////////////////////////////////////////////////////
// WAITFORTIMELINETOLOSEFOCUS
//////////////////////////////////////////////////////////////////////////

WaitForTimelineToLoseFocus::WaitForTimelineToLoseFocus()
    :   mFound(false)
{
    ASSERT( FixtureGui::UseRealUiEvents); // Otherwise timeline never has the focus anyway
#ifdef _MSC_VER
    getTimeline().Bind(wxEVT_LEAVE_WINDOW, &WaitForTimelineToLoseFocus::onLeave, this);
#else
    getTimeline().Bind(wxEVT_KILL_FOCUS,   &WaitForTimelineToLoseFocus::onFocus, this);
#endif
}

WaitForTimelineToLoseFocus::~WaitForTimelineToLoseFocus()
{
#ifdef _MSC_VER
    getTimeline().Unbind(wxEVT_LEAVE_WINDOW, &WaitForTimelineToLoseFocus::onLeave, this);
#else
    getTimeline().Unbind(wxEVT_KILL_FOCUS,   &WaitForTimelineToLoseFocus::onFocus, this);
#endif
}

void WaitForTimelineToLoseFocus::wait()
{
    boost::mutex::scoped_lock lock(mMutex);
    if (!mFound)
    {
        mCondition.wait(lock);
    }
}

void WaitForTimelineToLoseFocus::onLeave(wxMouseEvent& event)
{
    endWait();
    event.Skip();
}

void WaitForTimelineToLoseFocus::onFocus(wxFocusEvent& event)
{
    endWait();
    event.Skip();
}

void WaitForTimelineToLoseFocus::endWait()
{
    boost::mutex::scoped_lock lock(mMutex);
    mFound = true;
    mCondition.notify_all();
}

} // namespace
