#include "Drag.h"

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <wx/config.h>
#include <wx/dnd.h>
#include <wx/pen.h>
#include <wx/tooltip.h>
#include "AudioClip.h"
#include "AudioFile.h"
#include "AudioTrack.h"
#include "Clip.h"
#include "ClipView.h"
#include "Config.h"
#include "Cursor.h"
#include "DataObject.h"
#include "EmptyClip.h"
#include "EventDrag.h"
#include "File.h"
#include "Layout.h"
#include "MousePointer.h"
#include "PositionInfo.h"
#include "ProjectView.h"
#include "Scrolling.h"
#include "Selection.h"
#include "Sequence.h"
#include "SequenceView.h"
#include "State.h"
#include "Timeline.h"
#include "Track.h"
#include "TrackCreator.h"
#include "TrackView.h"
#include "Transition.h"
#include "UtilInt.h"
#include "UtilList.h"
#include "UtilSet.h"
#include "UtilLogStl.h"
#include "UtilLogWxwidgets.h"
#include "VideoClip.h"
#include "VideoFile.h"
#include "VideoTrack.h"
#include "ViewMap.h"
#include "Zoom.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// HELPER CLASSES
//////////////////////////////////////////////////////////////////////////

/// Helper class required to receive DND event from wxWidgets, in case new assets are
/// being dragged into the timeline (for instance, originating from the Project View).
class DropTarget
    :   public Part
    ,   public wxDropTarget
{
public:
    DropTarget(Timeline* timeline)
        :   Part(timeline)
        ,   wxDropTarget(new DataObject())
    {
    }
    ~DropTarget()
    {
    }
    wxDragResult OnData (wxCoord x, wxCoord y, wxDragResult def)
    {
        return def;
    };
    wxDragResult OnEnter (wxCoord x, wxCoord y, wxDragResult def)
    {
        ProjectViewDropSource::current().setFeedback(false);
        getStateMachine().process_event(state::EvDragEnter(x,y));
        return wxDragNone;
    }
    wxDragResult OnDragOver (wxCoord x, wxCoord y, wxDragResult def)
    {
        getStateMachine().process_event(state::EvDragMove(x,y));
        return wxDragMove;
    }
    bool OnDrop (wxCoord x, wxCoord y)
    {
        getStateMachine().process_event(state::EvDragDrop(x,y));
        return true;
    }
    void OnLeave()
    {
        ProjectViewDropSource::current().setFeedback(true);
        getStateMachine().process_event(state::EvDragEnd(0,0));
    }
};

/// Dummy class to be able to create views for tracks and clips in case of adding them from the project view.
/// This is a 'top' view class that ignores all events.
class DummyView : public View
{
public:
    DummyView(Timeline* timeline) : View(timeline) {}
    ~DummyView() {}
    wxSize requiredSize() const { FATAL; return wxSize(0,0); }
    void draw(wxBitmap& bitmap) const { FATAL; }
};

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Drag::Drag(Timeline* timeline)
    :   Part(timeline)
    ,   mCommand(0)
    ,   mIsInsideDrag(true)
    ,   mHotspot(0,0)
    ,   mPosition(0,0)
    ,   mBitmapOffset(0,0)
    ,   mBitmap()
    ,   mActive(false)
    ,   mDraggedTrack()
    ,   mDropTrack()
    ,   mVideo(timeline, true)
    ,   mAudio(timeline, false)
    ,   mShift(boost::none)
{
    VAR_DEBUG(this);
    getTimeline().SetDropTarget(new DropTarget(timeline)); // Drop target is deleted by wxWidgets
}

Drag::~Drag()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// START/STOP
//////////////////////////////////////////////////////////////////////////

void Drag::start(wxPoint hotspot, bool isInsideDrag)
{
    PointerPositionInfo info = getMousePointer().getInfo(hotspot);

    reset();
    mActive = true;
    mIsInsideDrag = isInsideDrag;
    mHotspot = hotspot;
    mHotspotPts = getZoom().pixelsToPts(mHotspot.x);
    mPosition = hotspot;
    mBitmapOffset = wxPoint(0,0);
    mDropTrack = info.track;
    mCommand = new command::ExecuteDrop(getSequence());
    command::ExecuteDrop::Drags drags;

    if (!mIsInsideDrag)
    {
        ::command::TrackCreator c(ProjectViewDropSource::current().getData().getAssets());
        mVideo.setTempTrack(c.getVideoTrack());
        mAudio.setTempTrack(c.getAudioTrack());
        mDraggedTrack = mVideo.getTempTrack();
        mHotspot.x = getZoom().ptsToPixels(mVideo.getTempTrack()->getLength() / 2);
        mHotspotPts = getZoom().pixelsToPts(mHotspot.x);

        // When dragging new clips into the timeline, the clips also need to be removed first.
        // This ensures that any used View classes are destroyed. Otherwise, there remain multiple
        // Views for a clip.
        UtilSet<model::IClipPtr>(drags).addElements(mVideo.getTempTrack()->getClips());
        UtilSet<model::IClipPtr>(drags).addElements(mAudio.getTempTrack()->getClips());
    }
    else
    {
        mDraggedTrack = info.track;
        UtilSet<model::IClipPtr>(drags).addElements(getSequence()->getSelectedClips());
    }

    ASSERT(mCommand);
    mCommand->onDragStart(drags, mIsInsideDrag);

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
    mBitmap = getDragBitmap();
    move(mHotspot);
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

    PointerPositionInfo info = getMousePointer().getInfo(position);

    if (wxGetMouseState().ControlDown())
    {
        // As long as CTRL is down, the dragged image stays the same, but the hotspot is moved
        mHotspot -=  mPosition - position;
        mHotspotPts = getZoom().pixelsToPts(mHotspot.x);
        updateDraggedTrack(info.track);
    }
    else if (!info.track || info.track == mDropTrack)
    {
        // Mouse is moved within the current track.
        // No changes in mDraggedTrack or mDroppedTrack required
        //
        // Move the cursor without moving the dragged object (note: vertical only!)
        mHotspot.y -= mPosition.y - position.y;
    }
    else
    {
        // The pointer is moved to another track.
        if (info.track->isA<model::VideoTrack>() == mDraggedTrack->isA<model::VideoTrack>())
        {
            // The pointer moved between video tracks or between audio tracks.
            updateOffset(info.track);
        }
        else
        {
            // Pointer moved from video to audio. The 'dragged track' must be updated.
            // The offsets are not changed. These are only changed when moving a track
            // of type x to another track of type x.
            updateDraggedTrack(info.track);
        }
        mHotspot.y = position.y;
        mBitmap = getDragBitmap();
    }

    mDropTrack = info.track;
    mPosition = position;
    redrawRegion.Union(wxRect(mBitmapOffset + mPosition + getSnapPixels() - mHotspot - scroll, mBitmap.GetSize())); // Redraw the new area (moved 'into' this area)

    // Snapping determination
    std::list<pts> prevsnaps = mSnaps;
    determineSnapOffset();
    BOOST_FOREACH( pts snap, prevsnaps )
    {
        if (!UtilList<pts>(mSnaps).hasElement(snap))
        {
            getTimeline().refreshPts(snap);
        }
    }
    BOOST_FOREACH( pts snap, mSnaps )
    {
        if (!UtilList<pts>(prevsnaps).hasElement(snap))
        {
            getTimeline().refreshPts(snap);
        }
    }

    // Shift if required
    determineShift();

    wxRegionIterator it(redrawRegion);
    while (it)
    {
        getTimeline().RefreshRect(it.GetRect(), false);
        it++;
    }

    VAR_DEBUG(*this);
}

void Drag::drop()
{
    VAR_DEBUG(*this);
    command::ExecuteDrop::Drops drops;
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getTracks() )
    {
        drops.splice(drops.end(), getDrops(track));
    }

    mCommand->onDrop(drops, mShift);
    mCommand->submit();
    mCommand = 0;
}

void Drag::stop()
{
    VAR_DEBUG(*this);
    mActive = false;            // Ensure that moved clips are not blanked out anymore. See ClipView::draw().
    mShift = boost::none;
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getTracks() )
    {
        getViewMap().getView(track)->onShiftChanged();
    }
    if (mCommand) // Was not reset in 'drop()', therefore the draganddrop was aborted
    {
        mCommand->onAbort();
        delete mCommand;
        mCommand = 0;
    }
    reset();
    getTimeline().Refresh(false);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

bool Drag::isActive() const
{
    return mActive;
}

bool Drag::contains(model::IClipPtr clip) const
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

//////////////////////////////////////////////////////////////////////////
// DRAW
//////////////////////////////////////////////////////////////////////////

wxBitmap Drag::getDragBitmap() //const
{
    VAR_DEBUG(*this);
    wxSize size = getSequenceView().getSize();

    wxBitmap temp(size); // Create a bitmap equal in size to the entire virtual area (for simpler drawing code)
    wxBitmap mask(size,1);

    wxMemoryDC dc(temp); // Must go out of scope to be able to use temp.data below
    wxMemoryDC dcMask(mask);

    dc.SetBackground(*wxTRANSPARENT_BRUSH);
    dc.Clear();
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(wxPoint(0,0),size);

    dcMask.SetBackground(*wxBLACK_BRUSH);
    dcMask.Clear();
    dcMask.SetPen(*wxWHITE_PEN);
    dcMask.SetBrush(*wxWHITE_BRUSH);

    // Draw video tracks
    wxPoint position(0,getSequenceView().getVideoPosition());
    BOOST_REVERSE_FOREACH( model::TrackPtr track, getSequence()->getVideoTracks() )
    {
        position.y += Layout::sTrackDividerHeight;
        model::TrackPtr draggedTrack = trackOnTopOf(track);
        if (draggedTrack)
        {
            getViewMap().getView(draggedTrack)->drawForDragging(position,track->getHeight(),dc,dcMask);
        }
        position.y += track->getHeight();
    }

    // Draw audio tracks
    position.y = getSequenceView().getAudioPosition();
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getAudioTracks() )
    {
        model::TrackPtr draggedTrack = trackOnTopOf(track);
        if (draggedTrack)
        {
            getViewMap().getView(draggedTrack)->drawForDragging(position,track->getHeight(),dc,dcMask);
        }
        position.y += track->getHeight() + Layout::sTrackDividerHeight;
    }

    mBitmapOffset.x = std::max(dcMask.MinX(),0);
    mBitmapOffset.y = std::max(dcMask.MinY(),0);

    int size_x = dcMask.MaxX() - mBitmapOffset.x;
    int size_y = dcMask.MaxY() - mBitmapOffset.y;

    dc.SelectObject(wxNullBitmap);
    dcMask.SelectObject(wxNullBitmap);

    temp.SetMask(new wxMask(mask));

    VAR_DEBUG(mBitmapOffset)(size_x)(size_y);
    ASSERT_MORE_THAN_ZERO(size_x);
    ASSERT_MORE_THAN_ZERO(size_y);
    return temp.GetSubBitmap(wxRect(mBitmapOffset.x,mBitmapOffset.y,size_x,size_y));
}

void Drag::draw(wxDC& dc) const
{
    if (!mActive)
    {
        return;
    }
    dc.DrawBitmap(mBitmap, getBitmapPosition(),true);
    dc.SetPen(Layout::sSnapPen);
    dc.SetBrush(Layout::sSnapBrush);
    BOOST_FOREACH( pts snap, mSnaps )
    {
        dc.DrawLine(getZoom().ptsToPixels(snap),0,getZoom().ptsToPixels(snap),dc.GetSize().GetHeight());
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
    mMinOffset = 0;
    mMaxOffset = 0;

    if (mTempTrack)
    {
        // Remove previous track
        delete getViewMap().getView(mTempTrack);
    }
    mTempTrack.reset();

    // Determine boundaries for 'inside' drags
    std::set<model::TrackPtr> selectedTracks;
    BOOST_FOREACH( model::IClipPtr clip, getSequence()->getSelectedClips() )
    {
        model::TrackPtr track = clip->getTrack();
        if (track->isA<model::VideoTrack>() == mIsVideo)
        {
            mMinOffset = std::min(mMinOffset, track->getIndex() * -1);
            mMaxOffset = std::max(mMaxOffset, nTracks() - track->getIndex());
        }
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

model::TrackPtr Drag::DragInfo::getTrack(int index)
{
    return  mIsVideo ? getSequence()->getVideoTrack(index) : getSequence()->getAudioTrack(index);
}

model::TrackPtr Drag::DragInfo::getTempTrack()
{
    return mTempTrack;
}

void Drag::DragInfo::setTempTrack(model::TrackPtr track)
{
    if (mTempTrack)
    {
        // Remove previous track
        delete getViewMap().getView(mTempTrack);
    }
    mTempTrack = track;
    new TrackView(mTempTrack, mView);
}

model::TrackPtr Drag::DragInfo::trackOnTopOf(model::TrackPtr track)
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

model::TrackPtr Drag::DragInfo::trackUnder(model::TrackPtr draggedtrack)
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
    mShift = boost::none;
}

model::TrackPtr Drag::trackOnTopOf(model::TrackPtr track)
{
    return getAssociatedInfo(track).trackOnTopOf(track);
}

model::TrackPtr Drag::trackUnder(model::TrackPtr draggedtrack)
{
    return getAssociatedInfo(draggedtrack).trackUnder(draggedtrack);
}

Drag::DragInfo& Drag::getAssociatedInfo(model::TrackPtr track)
{
    return track->isA<model::VideoTrack>() ? mVideo : mAudio;
}

void Drag::updateOffset(model::TrackPtr trackUnderPointer)
{
    getAssociatedInfo(trackUnderPointer).updateOffset(trackUnderPointer->getIndex(), mDraggedTrack->getIndex());
}

void Drag::updateDraggedTrack(model::TrackPtr track)
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
    pts minDiff = Layout::sSnapDistance + 1; // To ensure that the first found point will change this value
    pts snapPoint = -1;
    pts snapOffset = 0;
    std::list<pts>::const_iterator itTimeline = mSnapPoints.begin();
    std::list<pts>::const_iterator itDrag = mDragPoints.begin();
    while ( itTimeline != mSnapPoints.end() && itDrag != mDragPoints.end() )
    {
        pts pts_timeline = *itTimeline;
        pts pts_drag = *itDrag + ptsoffset;

        pts diff = abs(pts_drag - pts_timeline);
        if (diff <= Layout::sSnapDistance)
        {
            // This snap point is closer than the currently stored snap point, or it is equally
            // close, but is closer to the mouse pointer.
            if ((diff < minDiff) ||
                ((diff == minDiff) && (abs(pts_drag - ptsmouse) < abs(snapPoint - ptsmouse))))
            {
                minDiff = diff;
                snapPoint = pts_timeline;
                snapOffset = pts_timeline - pts_drag;
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

    VAR_DEBUG(snapPoint)(snapOffset);
    mSnapOffset = snapOffset;

    // Now determine all 'snaps' (positions where dragged cuts and timeline cuts are aligned)
    mSnaps.clear();
    itTimeline = mSnapPoints.begin();
    itDrag = mDragPoints.begin();
    while ( itTimeline != mSnapPoints.end() && itDrag != mDragPoints.end() )
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

    if (model::Config::ReadBool(model::Config::sPathSnapClips))
    {
        BOOST_FOREACH( model::TrackPtr track, getSequence()->getTracks() )
        {
            BOOST_FOREACH( model::IClipPtr clip, track->getClips() )
            {
                if (!contains(clip))
                {
                    mSnapPoints.push_back(clip->getLeftPts());
                    mSnapPoints.push_back(clip->getRightPts());
                }
            }
        }
    }
    if (model::Config::ReadBool(model::Config::sPathSnapCursor))
    {
        mSnapPoints.push_back(getZoom().pixelsToPts(getCursor().getPosition()));
    }

    mSnapPoints.sort();
    mSnapPoints.unique();
}

void Drag::determinePossibleDragPoints()
{
    mDragPoints.clear();

    BOOST_FOREACH( model::IClipPtr clip, mCommand->getDrags() )
    {
        mDragPoints.push_back(clip->getLeftPts());
        mDragPoints.push_back(clip->getRightPts());
    }

    mDragPoints.sort();
    mDragPoints.unique();
}

void Drag::determineShift()
{
    Shift shift = boost::none;
    if (wxGetMouseState().ShiftDown())
    {
        pts origPos = getDragPtsPosition();
        pts origLen = getDragPtsSize();

        pts pos = getDragPtsPosition();
        pts len = getDragPtsSize();

        BOOST_FOREACH( model::TrackPtr track, getSequence()->getTracks() )
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
            if ((clip) &&                               // Maybe no clip at given position
                (!clip->isA<model::EmptyClip>()) &&     // See remark above
                (clip->getLeftPts() < pos))             // Start of this clip is to the left of the currently calculated shift start position
            {
                pos = clip->getLeftPts();               // New shift start position: shift this clip entirely
                len =
                    getDragPtsSize() +                  // The original shift length
                    (origPos - clip->getLeftPts());     // The part of the clip that is left of that position must be shifted also
                // Note: do not make the mistake of using 'pos' here (origPos is used, since also the original drag size getDragSize() is used).
            }
        }
        shift.reset(ShiftParams(pos,len));
    }
    if (shift != mShift)
    {
        mShift = shift;
        VAR_DEBUG(shift);
        BOOST_FOREACH( model::TrackPtr track, getSequence()->getTracks() )
        {
            getViewMap().getView(track)->onShiftChanged();
        }
    }
}

command::ExecuteDrop::Drops Drag::getDrops(model::TrackPtr track)
{
    command::ExecuteDrop::Drops drops;
    model::TrackPtr draggedTrack = trackOnTopOf(track);
    if (draggedTrack)
    {
        pts position = 0;
        command::ExecuteDrop::Drop pi;
        pi.position = -1;
        pi.track = track;
        bool inregion = false;

        BOOST_FOREACH( model::IClipPtr clip, draggedTrack->getClips() )
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

std::ostream& operator<< (std::ostream& os, const Drag& obj)
{
    os  << &obj                 << '|'
        << obj.mIsInsideDrag    << '|'
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

std::ostream& operator<< (std::ostream& os, const Drag::DragInfo& obj)
{
    os  << obj.mIsVideo     << '|'
        << obj.mOffset      << '|'
        << obj.mMinOffset   << '|'
        << obj.mMaxOffset   << '|'
        << obj.mTempTrack;
    return os;
}
}} // namespace