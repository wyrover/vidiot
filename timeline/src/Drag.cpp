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

#include "Drag.h"

#include "AudioClip.h"
#include "AudioFile.h"
#include "AudioTrack.h"
#include "AudioView.h"
#include "Clip.h"
#include "ClipView.h"
#include "Config.h"
#include "CreateAudioTrack.h"
#include "CreateVideoTrack.h"
#include "Cursor.h"
#include "DividerView.h"
#include "Drag_Shift.h"
#include "EmptyClip.h"
#include "ExecuteDrop.h"
#include "EventDrag.h"
#include "EventKey.h"
#include "ExecuteDrop.h"
#include "File.h"
#include "Keyboard.h"
#include "Logging.h"
#include "Mouse.h"
#include "PositionInfo.h"
#include "ProjectView.h"
#include "ProjectViewDataObject.h"
#include "Scrolling.h"
#include "Selection.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "State.h"
#include "Timeline.h"
#include "TimelineDropTarget.h"
#include "Track.h"
#include "TrackView.h"
#include "Transition.h"
#include "UtilInt.h"
#include "UtilLogStl.h"
#include "UtilLogWxwidgets.h"
#include "UtilSet.h"
#include "UtilVector.h"
#include "VideoClip.h"
#include "VideoFile.h"
#include "VideoTrack.h"
#include "VideoView.h"
#include "ViewMap.h"
#include "Zoom.h"

namespace gui { namespace timeline {

const pixel Drag::Threshold{ 2 };

//////////////////////////////////////////////////////////////////////////
// HELPER CLASSES
//////////////////////////////////////////////////////////////////////////

/// Dummy class to be able to create views for tracks and clips in case of adding them from the project view.
/// This is a 'top' view class that ignores all events.
class DummyView : public View
{
public:
    DummyView(Timeline* timeline) : View(timeline) {}
    ~DummyView() {}
    pixel getX() const override { return getSequenceView().getX(); }
    pixel getY() const override { return getSequenceView().getY(); }
    pixel getW() const override { FATAL; return 0; }
    pixel getH() const override { FATAL; return 0; }
    void draw(wxDC& dc, const wxRegion& region, const wxPoint& offset) const override { FATAL; };
    void invalidateRect() override { FATAL; }
};

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Drag::Drag(Timeline* timeline)
    :   Part(timeline)
    ,   mCommand(0)
    ,   mHotspot(0,0)
    ,   mPosition(0,0)
    ,   mBitmapOffset(0,0)
    ,   mBitmap()
    ,   mActive(false)
    ,   mDraggedTrack()
    ,   mDropTrack()
    ,   mVideo(timeline, true)
    ,   mAudio(timeline, false)
    ,   mShift()
    ,   mSnappingEnabled(false)
{
    VAR_DEBUG(this);
    getTimeline().SetDropTarget(new TimelineDropTarget(timeline)); // Drop target is deleted by wxWidgets
}

Drag::~Drag()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// START/STOP
//////////////////////////////////////////////////////////////////////////

void Drag::start(const wxPoint& hotspot, bool external)
{
    PointerPositionInfo info = getMouse().getInfo(hotspot);

    reset();
    mActive = true;
    mSnappingEnabled = true;
    mHotspot = hotspot;
    mHotspotPts = getZoom().pixelsToPts(mHotspot.x);
    mPosition = hotspot;
    mBitmapOffset = wxPoint(0,0);
    mDropTrack = info.track;
    mCommand = new command::ExecuteDrop(getSequence(), external);
    command::Drags drags;

    if (external)
    {
        TimelineDropTarget* dropTarget = dynamic_cast<TimelineDropTarget*>(getTimeline().GetDropTarget());
        ASSERT_NONZERO(dropTarget);
        ASSERT_NONZERO(dropTarget->getVideo());
        ASSERT_NONZERO(dropTarget->getAudio());
        ASSERT_EQUALS(dropTarget->getVideo()->getLength(), dropTarget->getAudio()->getLength());
        ASSERT_NONZERO(dropTarget->getVideo()->getLength());
        mVideo.setTempTrack(dropTarget->getVideo());
        mAudio.setTempTrack(dropTarget->getAudio());

        mDraggedTrack = mVideo.getTempTrack();

        if (getSequence()->getLength() < mVideo.getTempTrack()->getLength())
        {
            // Extend sequence view if it is not big enough to hold the dragged data.
            getSequenceView().setMinimumLength(getSequenceView().getDefaultLength() + mVideo.getTempTrack()->getLength());
        }

        mHotspot.x = getZoom().ptsToPixels(mVideo.getTempTrack()->getLength() / 2);
        mHotspotPts = getZoom().pixelsToPts(mHotspot.x);

        // When dragging new clips into the timeline, the clips also need to be removed first.
        // This ensures that any used View classes are destroyed. Otherwise, there remain multiple
        // Views for a clip.
        UtilSet<model::IClipPtr>(drags).addElements(mVideo.getTempTrack()->getClips());
        UtilSet<model::IClipPtr>(drags).addElements(mAudio.getTempTrack()->getClips());
        // EmptyClip areas must not be dragged along.
        for (auto it = drags.begin(); it != drags.end();)
        {
            if ((*it)->isA<model::EmptyClip>())
            {
                it = drags.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
    else
    {
        mDraggedTrack = info.track;
        UtilSet<model::IClipPtr>(drags).addElements(getSequence()->getSelectedClips());
    }

    ASSERT(mCommand);
    mCommand->onDragStart(drags);

    VAR_DEBUG(*this);
    ASSERT(mDraggedTrack);

    determinePossibleSnapPoints();
    determinePossibleDragPoints(); // Requires initialized getDrags() in mCommand
    show();
}

void Drag::show()
{
    // mHotspotPts must be aligned with pixelsToPts(position)
    mHotspot.x = getZoom().ptsToPixels(mHotspotPts);
    updateDragBitmap();
    move(mHotspot);
}

void Drag::toggleSnapping()
{
    mSnappingEnabled = !mSnappingEnabled;
    move(getMouse().getVirtualPosition());
}

void Drag::move(wxPoint position)
{
    VAR_DEBUG(*this);

    wxPoint scroll = getScrolling().getOffset();

    wxRegion redrawRegion(wxRect(mBitmapOffset + mPosition + getSnapPixels() - mHotspot - scroll, mBitmap.GetSize())); // Redraw the old area (moved 'out' of this area)

    if (position.x - mHotspot.x + mBitmapOffset.x < 0)
    {
        position.x = mHotspot.x - mBitmapOffset.x; // Can't move 'beyond 0'
    }

    PointerPositionInfo info = getMouse().getInfo(position);

    wxPoint previousHotSpot(mHotspot);
    if (getKeyboard().getCtrlDown())
    {
        // As long as CTRL is down, the vertical position of the hotspot is moved.
        //
        // Can be used to position the mouse on another track (for instance, audio vs.
        // video). Example: drag video clip from track 0 to video track 1, and drag the
        // linked audio clip to audio track 1 also.
        //
        // Note that horizontal movement still results in drag movement.
        // Rationale: due to the clipping for the dragged bitmap (not starting with
        //            the maximum virtual size of the timeline but only maximally
        //            2x the client area size of the timeline), moving the hotspot
        //            horizontally can 'move in' part of the dragged clips that are
        //            not part of the drag bitmap anymore.
        mHotspot.y -=  mPosition.y - position.y;
        mHotspotPts = getZoom().pixelsToPts(mHotspot.x);
        updateDraggedTrack(info.track);
    }
    else if (!info.track || info.track == mDropTrack)
    {
        model::TrackPtr addedTrack = nullptr;
        if (Config::ReadBool(Config::sPathTimelineAutoAddEmptyTrackWhenDragging))
        {
            if (!info.track && position.y < getSequenceView().getVideo().getRect().GetTop())
            {
                // Add a new video track if the mouse pointer is moved 'above' all video tracks.
                if (mCommand->isVideoClipDragged())
                {
                    // But only add the track if a video clip is being dragged
                    addedTrack = mCommand->onAddTrack(true);
                }
            }
            else if (!info.track && position.y > getSequenceView().getAudio().getRect().GetBottom())
            {
                // Add a new audio track if the mouse pointer is moved 'below' all audio tracks.
                if (mCommand->isAudioClipDragged())
                {
                    // But only add the track if an audio clip is being dragged
                    addedTrack = mCommand->onAddTrack(false);
                }
            }
        }
        if (addedTrack)
        {
            updateOffset(addedTrack); // Dragged clip(s) are positioned in the new track.
            updateDragBitmap(); // New track is added, so the vertical offsets have changed.
            getTimeline().Refresh(); // Due to adding a video track at the top, all others shift downwards.
        }

        // Mouse is moved, but hotspot stays within the current track.
        // No changes in mDraggedTrack required
        //
        // Move the pointer position without moving the dragged object (note: vertical only!)
        mHotspot.y -= mPosition.y - position.y;
    }
    else
    {
        // The pointer is moved to another track.
        if (info.track->isA<model::VideoTrack>() == mDraggedTrack->isA<model::VideoTrack>())
        {
            // The pointer moved between video tracks or between audio tracks.
            updateOffset(info.track); // Can be updated BEFORE adding/removing tracks, since info.track will always be lower than the added/removed track,

            if (getAssociatedInfo(info.track).requiresAddedTrack())
            {
                // Add a track if the required number of tracks (for holding the drag) has become greater than the number of available tracks.
                mCommand->onAddTrack(info.track->isA<model::VideoTrack>());
            }
            else
            {
                // If the track created during dragging is no longer required to hold the dragged clips, remove it again.
                mCommand->onRemoveAddedTrack(info.track->isA<model::VideoTrack>());
            }
        }
        else
        {
            // Pointer moved from video to audio. The 'dragged track' must be updated.
            // The offsets are not changed. These are only changed when moving a track
            // of type x to another track of type x.
            updateDraggedTrack(info.track);
        }
        mHotspot.y = position.y;
        updateDragBitmap();
    }

    mDropTrack = info.track;
    mPosition = position;

    // Snapping determination
    std::vector<pts> prevsnaps = mSnaps;
    determineSnapOffset();

    // Determine which regions of the timeline to update
    redrawRegion.Union(wxRect(mBitmapOffset + mPosition + getSnapPixels() - mHotspot - scroll, mBitmap.GetSize())); // Redraw the new area (moved 'into' this area)
    for ( pts snap : prevsnaps )
    {
        if (!UtilVector<pts>(mSnaps).hasElement(snap))
        {
            getTimeline().refreshPts(snap);
        }
    }
    for ( pts snap : mSnaps )
    {
        if (!UtilVector<pts>(prevsnaps).hasElement(snap))
        {
            getTimeline().refreshPts(snap);
        }
    }

    // Shift if required
    bool hadShift = mShift != nullptr;
    determineShift();
    bool hasShift = mShift != nullptr;
    if (hasShift || hadShift)
    {
        getTimeline().Refresh(false);
    }
    else
    {
        wxRegionIterator it(redrawRegion);
        while (it)
        {
            getTimeline().RefreshRect(it.GetRect(), false);
            it++;
        }
    }

    VAR_DEBUG(*this);
}

void Drag::drop()
{
    VAR_DEBUG(*this);
    command::Drops drops;
    for ( model::TrackPtr track : getSequence()->getTracks() )
    {
        command::Drops adddrops = getDrops(track);
        VAR_INFO(track)(adddrops);
        drops.insert(drops.end(), adddrops.begin(), adddrops.end());
    }

    mCommand->onDrop(drops, mShift);
    mCommand->submit();
    mCommand = 0;
}

void Drag::stop()
{
    VAR_DEBUG(*this);
    mActive = false;            // Ensure that moved clips are not blanked out anymore. See ClipView::draw().
    mShift.reset();
    if (mCommand) // Was not reset in 'drop()', therefore the draganddrop was aborted
    {
        mCommand->onAbort();
        delete mCommand;
        mCommand = 0;
    }
    reset();
    getSequenceView().setMinimumLength(0);
    getTimeline().resize();
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

bool Drag::isActive() const
{
    return mActive;
}

bool Drag::contains(const model::IClipPtr& clip) const
{
    ASSERT(mCommand);
    return mCommand->getDrags().find(clip) != mCommand->getDrags().end();
}

wxSize Drag::getBitmapSize() const
{
    return mBitmap.GetSize();
}

wxPoint Drag::getBitmapPosition() const
{
    ASSERT(isActive());
    return mBitmapOffset + getDraggedDistance() + getSnapPixels();
}

Shift Drag::getShift() const
{
    return mShift;
}

pts Drag::getSnapOffset() const
{
    return mSnapOffset;
}

//////////////////////////////////////////////////////////////////////////
// DRAW
//////////////////////////////////////////////////////////////////////////

void Drag::updateDragBitmap()
{
    VAR_DEBUG(*this);
    // X region of the drag bitmap initially contains one time the entire timeline client area
    // to the left of the hotspot, and one time the entire timeline client width to the right of
    // the hotspot x position. In that way, there's just enough to fully drag to the leftmost
    // pixel of the client area or drag to the rightmost pixel of the client area.
    //
    // Note that the area around the hotspot is used and not the area currently visible:
    // The to-be-dragged clips are not always in the visible region, particularly when
    // using right-mouse-scrolling during the drag and drop operation.
    pixel dragBitmapOffsetX = std::max(0, mHotspot.x - getTimeline().GetClientSize().GetWidth());
    pixel dragBitmapOffsetY = getSequenceView().getVideo().getY();
    wxSize bitmapSize{ std::min(2 * getTimeline().GetClientSize().GetWidth(), getTimeline().GetVirtualSize().GetWidth()), getSequenceView().getH() };

    wxBitmap temp(bitmapSize);
    wxBitmap mask(bitmapSize,1);

    wxMemoryDC dc(temp);
    wxMemoryDC dcMask(mask);

    wxBrush transparentBrush{wxColour{0,0,0}, wxBRUSHSTYLE_TRANSPARENT};
    dc.SetBackground(transparentBrush);
    dc.Clear();
    dc.SetBrush(transparentBrush);
    dc.SetPen(wxPen{wxColour{0,0,0}, 1, wxPENSTYLE_TRANSPARENT});
    dc.DrawRectangle(wxPoint(0,0),bitmapSize);

    dcMask.SetBackground(wxBrush{wxColour{0,0,0}});
    dcMask.Clear();
    dcMask.SetPen(wxPen{wxColour{255,255,255}});
    dcMask.SetBrush(wxBrush{wxColour{255,255,255}});

    // Draw video tracks
    wxPoint position(-dragBitmapOffsetX, getSequenceView().getVideo().getY() - dragBitmapOffsetY);
    model::Tracks videoTracks = getSequence()->getVideoTracks(); // Can't use reverse on temporary inside for loop
    for ( model::TrackPtr track : boost::adaptors::reverse( videoTracks ) )
    {
        position.y += DividerView::TrackDividerHeight;
        model::TrackPtr draggedTrack = trackOnTopOf(track);
        if (draggedTrack)
        {
            getViewMap().getView(draggedTrack)->drawForDragging(position,track->getHeight(),dc,dcMask);
        }
        position.y += track->getHeight();
    }

    // Draw audio tracks
    position.y = getSequenceView().getAudio().getY() - dragBitmapOffsetY;
    for ( model::TrackPtr track : getSequence()->getAudioTracks() )
    {
        model::TrackPtr draggedTrack = trackOnTopOf(track);
        if (draggedTrack)
        {
            getViewMap().getView(draggedTrack)->drawForDragging(position,track->getHeight(),dc,dcMask);
        }
        position.y += track->getHeight() + DividerView::TrackDividerHeight;
    }

    pixel roi_x = std::max(dcMask.MinX(),0);
    pixel roi_y = std::max(dcMask.MinY(),0);

    int size_x = std::min(bitmapSize.GetWidth(),  dcMask.MaxX()) - roi_x;
    int size_y = std::min(bitmapSize.GetHeight(), dcMask.MaxY()) - roi_y;

    dc.SelectObject(wxNullBitmap);
    dcMask.SelectObject(wxNullBitmap);

    temp.SetMask(new wxMask(mask));

    VAR_DEBUG(bitmapSize)(roi_x)(roi_y)(size_x)(size_y);
    ASSERT_MORE_THAN_ZERO(size_x);
    ASSERT_MORE_THAN_ZERO(size_y);
    mBitmapOffset.x = dragBitmapOffsetX + roi_x;
    mBitmapOffset.y = dragBitmapOffsetY + roi_y;

    ASSERT(temp.IsOk());
    ASSERT_MORE_THAN_EQUALS_ZERO(roi_x);
    ASSERT_MORE_THAN_EQUALS_ZERO(roi_y);
    ASSERT_LESS_THAN_EQUALS(roi_x + size_x, temp.GetWidth());
    ASSERT_LESS_THAN_EQUALS(roi_y + size_y, temp.GetHeight());
    // GetSubBitmap is not an obsolete call: the bitmap becomes smaller, although only
    // slightly. Furthermore, dragging to the left doesn't work, if the 'unclipped'
    // bitmap still touches the left edge of the timeline (low scrolling offset) 6.
    mBitmap =  temp.GetSubBitmap(wxRect(roi_x,roi_y,size_x,size_y));
}

void Drag::drawDraggedClips(wxDC& dc, const wxRegion& region, const wxPoint& offset) const
{
    if (mActive)
    {
        getTimeline().copyRect(dc,region,offset,mBitmap,wxRect(getBitmapPosition(),mBitmap.GetSize()),true);
    }
}

void Drag::drawSnaps(wxDC& dc, const wxRegion& region, const wxPoint& offset) const
{
    if (mActive)
    {
        for ( pts snap : mSnaps )
        {
            getTimeline().drawLine(dc, region, offset, snap, wxPen{ wxColour{ 164, 164, 164 } });
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// DRAGINFO
//////////////////////////////////////////////////////////////////////////

Drag::DragInfo::DragInfo(Timeline* timeline, bool isVideo)
:   Part(timeline)
,   mIsVideo(isVideo)
,   mOffset(0)
,   mMinOffset(0)
,   mMaxOffset(0)
,   mTempTrack()
,   mView(new DummyView(timeline))
{
}

Drag::DragInfo::~DragInfo()
{
    delete mView;
}

void Drag::DragInfo::reset()
{
    mOffset = 0;

    // -1: nTracks is 1-based
    mMinOffset = -1 * (nTracks() - 1);  
    
    // -1: nTracks is 1-based
    // -1: at least a clip in track 1 is selected so can be used as default.
    // +1: at most one track can be added automatically
    mMaxOffset = (nTracks() - 1) -1 + 1; 
    
    if (mTempTrack)
    {
        // Remove previous track
        delete getViewMap().getView(mTempTrack);
    }
    mTempTrack.reset();

    // Determine boundaries for 'inside' drags
    std::set<model::TrackPtr> selectedTracks;
    for ( model::IClipPtr clip : getSequence()->getSelectedClips() )
    {
        model::TrackPtr track = clip->getTrack();
        if (track->isA<model::VideoTrack>() == mIsVideo)
        {
            mMinOffset = std::max(mMinOffset, track->getIndex() * -1);
            mMaxOffset = std::min(mMaxOffset, nTracks() - track->getIndex() - 1); // -1: nTracks is 1-based, getIndex() is 0-based.
        }
    }

    if (!mTempTrack)
    {
        // One track may be added during the drag operation.
        // This is done for only internal drags since for external drags it leads to confusing behavior.
        mMaxOffset += 1; // now it works nicely for external drags, including adding the required track.
    }
}

void Drag::DragInfo::updateOffset(int indexOfTrackInTimeline, int indexOfDraggedTrack)
{
    if (!mTempTrack)
    {
        // 'inside' drag
        mOffset =
            std::min(mMaxOffset,
            std::max(mMinOffset,
            indexOfTrackInTimeline - indexOfDraggedTrack));
    }
    else
    {
        // 'outside' drag
        mOffset = indexOfTrackInTimeline;
    }
}

bool Drag::DragInfo::requiresAddedTrack() const
{
    return mOffset == mMaxOffset;
}

model::TrackPtr Drag::DragInfo::getTrack(int index)
{
    return  mIsVideo ? getSequence()->getVideoTrack(index) : getSequence()->getAudioTrack(index);
}

model::TrackPtr Drag::DragInfo::getTempTrack()
{
    return mTempTrack;
}

void Drag::DragInfo::setTempTrack(const model::TrackPtr& track)
{
    if (mTempTrack)
    {
        // Remove previous track
        delete getViewMap().getView(mTempTrack);
    }
    mTempTrack = track;
    new TrackView(mTempTrack, mView);
}

model::TrackPtr Drag::DragInfo::trackOnTopOf(const model::TrackPtr& track)
{
    VAR_DEBUG(track);
    model::TrackPtr draggedTrack;
    if (!mTempTrack)
    {
        // 'inside' drag
        draggedTrack = getTrack(track->getIndex() - mOffset);
    }
    else
    {
        // 'outside' drag
        if (mOffset == track->getIndex())
        {
            draggedTrack = mTempTrack;
        }
    }
    VAR_DEBUG(draggedTrack);
    return draggedTrack;
}

model::TrackPtr Drag::DragInfo::trackUnder(const model::TrackPtr& draggedtrack)
{
    VAR_DEBUG(draggedtrack);
    model::TrackPtr track;
    if (!mTempTrack)
    {
        track = getTrack(mOffset + draggedtrack->getIndex());
    }
    else
    {
        track = getTrack(mOffset);
    }
    VAR_DEBUG(track);
    return track;
}

int Drag::DragInfo::nTracks()
{
    return mIsVideo ? getSequence()->getVideoTracks().size() : getSequence()->getAudioTracks().size();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Drag::reset()
{
    delete mCommand;
    mHotspot = wxPoint(0,0);
    mHotspotPts = 0;
    mPosition = wxPoint(0,0);
    mBitmapOffset = wxPoint(0,0);
    mSnapPoints.clear();
    mDragPoints.clear();
    mSnapOffset = 0;
    mSnaps.clear();
    mVideo.reset();
    mAudio.reset();
    mDraggedTrack.reset();
    mDropTrack.reset();
    mShift.reset();
}

model::TrackPtr Drag::trackOnTopOf(const model::TrackPtr& track)
{
    return getAssociatedInfo(track).trackOnTopOf(track);
}

model::TrackPtr Drag::trackUnder(const model::TrackPtr& draggedtrack)
{
    return getAssociatedInfo(draggedtrack).trackUnder(draggedtrack);
}

Drag::DragInfo& Drag::getAssociatedInfo(const model::TrackPtr& track)
{
    return track->isA<model::VideoTrack>() ? mVideo : mAudio;
}

void Drag::updateOffset(const model::TrackPtr& trackUnderPointer)
{
    getAssociatedInfo(trackUnderPointer).updateOffset(trackUnderPointer->getIndex(), mDraggedTrack->getIndex());
}

void Drag::updateDraggedTrack(const model::TrackPtr& track)
{
    if (track)
    {
        model::TrackPtr newDraggedTrack = trackOnTopOf(track);
        if (newDraggedTrack)
        {
            mDraggedTrack = newDraggedTrack;
        }
    }
}

wxPoint Drag::getDraggedDistance() const
{
    return mPosition - mHotspot;
}

pts Drag::getDraggedPtsDistance() const
{
    return getZoom().pixelsToPts(getDraggedDistance().x);
}

pts Drag::getDragPtsPosition() const
{
    return getDraggedPosition(mDragPoints.front());
}

pts Drag::getDraggedPosition(pts dragpoint) const
{
    return dragpoint + getDraggedPtsDistance() + mSnapOffset;
}

pts Drag::getDragPtsSize() const
{
    return mDragPoints.back() - mDragPoints.front();
}

wxPoint Drag::getSnapPixels() const
{
    return wxPoint(getZoom().ptsToPixels(mSnapOffset),0);
}

void Drag::determineSnapOffset()
{
    pts ptsoffset = getDraggedPtsDistance();
    pts ptsmouse = getZoom().pixelsToPts(mPosition.x);

    // Find nearest snap match
    pts snapOffset = 0;
    pts snapPoint = -1;

    if (mSnappingEnabled)
    {
        pts minDiff = Timeline::SnapDistance + 1; // To ensure that the first found point will change this value
        std::vector<pts>::const_iterator itTimeline = mSnapPoints.begin();
        std::vector<pts>::const_iterator itDrag = mDragPoints.begin();
        ASSERT(itDrag != mDragPoints.end());
        pts leftMostDragPoint = *itDrag;

        while (itTimeline != mSnapPoints.end() && itDrag != mDragPoints.end())
        {
            pts pts_timeline = *itTimeline;
            pts pts_drag = *itDrag + ptsoffset;

            pts diff = abs(pts_drag - pts_timeline);
            if (diff <= Timeline::SnapDistance)
            {
                pts offset = pts_timeline - pts_drag;
                pts leftMostPointAfterDropping = leftMostDragPoint + offset + getDraggedPtsDistance();
                if (leftMostPointAfterDropping >= 0) // Avoid dropping 'before' position '0'.
                {
                    // The snapping may not cause the leftmost drag position to be moved before 0
                    if (((diff < minDiff) ||
                        ((diff == minDiff) && (abs(pts_drag - ptsmouse) < abs(snapPoint - ptsmouse)))))
                    {
                        // This snap point is closer than the currently stored snap point, or it is equally
                        // close, but is closer to the mouse pointer.
                        minDiff = diff;
                        snapPoint = pts_timeline;
                        snapOffset = offset;
                    }
                }

            }
            if (pts_timeline <= pts_drag)
            {
                ++itTimeline;
            }
            if (pts_timeline >= pts_drag)
            {
                ++itDrag;
            }
        }
    }

    // Always snap to '>=0', since positioning the drop < 0 is not allowed.
    // This is required for scenarios 
    // - where snapping is disabled, OR
    // - where no snaps are found (drag very large file)
    // (the checks in the loops are never encountered).
    std::vector<pts>::const_iterator itDrag = mDragPoints.begin();
    ASSERT(itDrag != mDragPoints.end());
    pts leftMostDragPoint = *itDrag;
    pts leftMostPointAfterDropping = leftMostDragPoint + snapOffset + getDraggedPtsDistance();
    if (leftMostPointAfterDropping < 0) // Avoid dropping 'before' position '0'.
    {
        snapOffset -= leftMostPointAfterDropping; // Move leftmost point to '0'.
    }

    VAR_DEBUG(snapPoint)(snapOffset);
    mSnapOffset = snapOffset;

    // Now determine all 'snaps' (positions where dragged cuts and timeline cuts are aligned)
    mSnaps.clear();
    std::vector<pts>::const_iterator itTimeline = mSnapPoints.begin();
    itDrag = mDragPoints.begin();
    while (itTimeline != mSnapPoints.end() && itDrag != mDragPoints.end())
    {
        pts pts_timeline = *itTimeline;
        pts pts_drag = *itDrag + ptsoffset + mSnapOffset;
        if (pts_timeline == pts_drag)
        {
            mSnaps.push_back(pts_timeline);
        }
        if (pts_timeline <= pts_drag)
        {
            ++itTimeline;
        }
        if (pts_timeline >= pts_drag)
        {
            ++itDrag;
        }
    }
}

void Drag::determinePossibleSnapPoints()
{
    mSnapPoints.clear();
    if (mSnappingEnabled && Config::ReadBool(Config::sPathTimelineSnapClips))
    {
        UtilVector<pts>(mSnapPoints).addElements(getSequence()->getCuts(mCommand->getDrags()));
    }
    if (mSnappingEnabled && Config::ReadBool(Config::sPathTimelineSnapCursor))
    {
        mSnapPoints.push_back(getCursor().getLogicalPosition());
    }

    std::sort(mSnapPoints.begin(), mSnapPoints.end());
    std::unique(mSnapPoints.begin(), mSnapPoints.end());

    VAR_DEBUG(mSnapPoints);
}

void Drag::determinePossibleDragPoints()
{
    mDragPoints.clear();

    for ( model::IClipPtr clip : mCommand->getDrags() )
    {
        mDragPoints.push_back(clip->getLeftPts());
        mDragPoints.push_back(clip->getRightPts());
    }

    std::sort(mDragPoints.begin(), mDragPoints.end());
    std::unique(mDragPoints.begin(), mDragPoints.end());
    VAR_DEBUG(mDragPoints);
}

void Drag::determineShift()
{
    Shift shift;
    if (getKeyboard().getShiftDown())
    {
        pts origPos = getDragPtsPosition();
        pts origLen = getDragPtsSize();

        pts pos = getDragPtsPosition();
        pts len = getDragPtsSize();

        for ( model::TrackPtr track : getSequence()->getTracks() )
        {
            model::IClipPtr clip = track->getClip(origPos);

            // Given the current estimated shift position, determine if for the given clip a bit of extra shift
            // is required, because the clip extends to the left of the shift position which must be shifted also.
            // For transitions, extra code is required, since the clip left/right of the transition are what the
            // user expects to be shifted. The actual transition itselves provides no 'shift' positions.
            // Finally, for empty clips, there is never additional RELEVANT clip to the left of the shift position
            // that needs to be shifted (shift means adding more empty clips which will be adjoined anyway).

            model::TransitionPtr transition = boost::dynamic_pointer_cast<model::Transition>(clip);
            if (transition)
            {
                // The drag is either over the left or the right part of the transition, depending on the
                // position where the two clips 'touch' from a user perspective.
                clip = (origPos < transition->getTouchPosition()) ? clip = clip->getPrev() : clip = clip->getNext();
                ASSERT(clip);
                ASSERT(!clip->isA<model::Transition>())(clip);
            }
            if (clip &&                                 // Maybe no clip at given position
                !clip->isA<model::EmptyClip>() &&       // See remark above
                !clip->getDragged() &&                  // Currently dragged clips are 'empty' during the drag
                (clip->getLeftPts() < pos))             // Start of this clip is to the left of the currently calculated shift start position
            {
                pos = clip->getLeftPts();               // New shift start position: shift this clip entirely
                len =
                    getDragPtsSize() +                  // The original shift length
                    (origPos - clip->getLeftPts());     // The part of the clip that is left of that position must be shifted also
                // Note: do not make the mistake of using 'pos' here (origPos is used, since also the original drag size getDragSize() is used).
            }
        }
        shift = boost::make_shared<ShiftParams>(getTimeline(),pos,len);
    }
    if (shift != mShift)
    {
        mShift = shift;
        getTimeline().resize();
    }
}

command::Drops Drag::getDrops(const model::TrackPtr& track)
{
    command::Drops drops;
    model::TrackPtr draggedTrack = trackOnTopOf(track);
    VAR_DEBUG(track)(draggedTrack);
    if (draggedTrack)
    {
        LOG_DEBUG << DUMP(track) << DUMP(draggedTrack);
        pts position = 0;
        command::Drop pi;
        pi.position = -1;
        pi.track = track;
        bool inregion = false;

        for ( model::IClipPtr clip : draggedTrack->getClips() )
        {
            if (!inregion && contains(clip))
            {
                inregion = true;
                pi.position = position + mSnapOffset + getDraggedPtsDistance();
            }
            if (inregion && !contains(clip))
            {
                inregion = false;
                drops.push_back(pi);
                pi.position = -1; // Prepare for new region
                pi.clips.clear(); // Prepare for new region
            }
            if (inregion)
            {
                pi.clips.push_back(clip);
            }
            position += clip->getLength();
        }
        if (inregion)
        {
            drops.push_back(pi); // Insertion at end
        }
    }
    return drops;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const Drag& obj)
{
    os  << &obj                 << '|'
        << obj.mHotspot         << '|'
        << obj.mHotspotPts      << '|'
        << obj.mPosition        << '|'
        << obj.mBitmapOffset    << '|'
        << obj.mActive          << '|'
        << obj.mSnapOffset      << '|'
        << obj.mShift           << '|'
        << obj.mVideo           << '|'
        << obj.mAudio           << '|'
        << obj.mDraggedTrack    << '|'
        << obj.mDropTrack       << '|';
    return os;
}

std::ostream& operator<<(std::ostream& os, const Drag::DragInfo& obj)
{
    os  << obj.mIsVideo     << '|'
        << obj.mOffset      << '|'
        << obj.mMinOffset   << '|'
        << obj.mMaxOffset   << '|'
        << obj.mTempTrack;
    return os;
}
}} // namespace
