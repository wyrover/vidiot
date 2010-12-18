#include "Drag.h"

#include <wx/pen.h>
#include <wx/tooltip.h>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include "Timeline.h"
#include "MousePointer.h"
#include "PositionInfo.h"
#include "Layout.h"
#include "TrackView.h"
#include "UtilLogWxwidgets.h"
#include "Track.h"
#include "Sequence.h"
#include "Selection.h"
#include "ViewMap.h"
#include "Divider.h"
#include "Clip.h"
#include "ClipView.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Drag::Drag(Timeline* timeline)
    :   Part(timeline)
    ,   mHotspot(0,0)
    ,   mPosition(0,0)
    ,   mBitmapOffset(0,0)
    ,   mBitmap()
    ,   mActive(false)
    ,   mDraggedTrack()
    ,   mDropTrack()
    ,   mVideo(timeline, true)
    ,   mAudio(timeline, false)
{
}

//////////////////////////////////////////////////////////////////////////
// START/STOP
//////////////////////////////////////////////////////////////////////////

void Drag::Start(wxPoint hotspot)
{
    PointerPositionInfo info = getMousePointer().getInfo(hotspot);

    mHotspot = hotspot;
    mPosition = hotspot;
    mBitmapOffset = wxPoint(0,0);
    mDraggedTrack = info.track;
    mDropTrack = info.track;
    mVideo.reset();
    mAudio.reset();
    VAR_DEBUG(*this);

    ASSERT(mDraggedTrack);

    mActive = true; // Must be done BEFORE getDragBitmap(), since it is used for creating that bitmap.

    // Redraw (hide) all selected clips in the timeline (to avoid them being shown on the timeline AND in the drag bitmap).
    BOOST_FOREACH( model::ClipPtr clip, getSelection().getClips() )
    {
        getViewMap().getView(clip)->invalidateBitmap();
    }
    mBitmap = getDragBitmap();
    MoveTo(hotspot, false);
}

void Drag::MoveTo(wxPoint position, bool altPressed)
{
    VAR_DEBUG(*this);
    wxRegion redrawRegion;

    redrawRegion.Union(wxRect(mBitmapOffset + mPosition - mHotspot, mBitmap.GetSize())); // Redraw the old area (moved 'out' of this area)

    PointerPositionInfo info = getMousePointer().getInfo(position);

    if (altPressed)
    {
        // As long as ALT is down, the dragged image stays the same, but the hotspot is moved
        mHotspot -=  mPosition - position;
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
        mDropTrack = info.track;

        if (info.track->isA<model::VideoTrack>() == mDraggedTrack->isA<model::VideoTrack>())
        {
            // The pointer moved between video tracks or between audio tracks.
            getAssociatedInfo(info.track).updateOffset(info.track->getIndex(), mDraggedTrack->getIndex());
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

    mPosition = position;
    redrawRegion.Union(wxRect(mBitmapOffset + mPosition - mHotspot, mBitmap.GetSize())); // Redraw the new area (moved 'into' this area)

    getTimeline().invalidateBitmap();

    wxRegionIterator it(redrawRegion);
    while (it)
    {
        getTimeline().RefreshRect(it.GetRect());
        it++;
    }

    VAR_DEBUG(*this);
}

void Drag::Stop()
{
    VAR_DEBUG(*this);
    mActive = false;
    BOOST_FOREACH( model::ClipPtr clip, getSelection().getClips() )
    {
        getViewMap().getView(clip)->invalidateBitmap();
    }
    getTimeline().Refresh();
    VAR_DEBUG(*this);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

bool Drag::isActive() const
{
    return mActive;
}

//////////////////////////////////////////////////////////////////////////
// DRAW
//////////////////////////////////////////////////////////////////////////

wxBitmap Drag::getDragBitmap() //const
{
    VAR_DEBUG(*this);
    int w = getTimeline().requiredWidth();
    int h = getTimeline().requiredHeight();

    wxBitmap temp(w,h); // Create a bitmap equal in size to the entire virtual area (for simpler drawing code)
    wxBitmap mask(w,h,1);

    wxMemoryDC dc(temp); // Must go out of scope to be able to use temp.data below
    wxMemoryDC dcMask(mask);

    dc.SetBackground(*wxTRANSPARENT_BRUSH);
    dc.Clear();
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.SetPen(*wxTRANSPARENT_PEN);
    dc.DrawRectangle(0,0,w,h);

    dcMask.SetBackground(*wxBLACK_BRUSH);
    dcMask.Clear();
    dcMask.SetPen(*wxWHITE_PEN);
    dcMask.SetBrush(*wxWHITE_BRUSH);

    // Draw video tracks
    wxPoint position(0,getDivider().getVideoPosition());
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
    position.y = getDivider().getAudioPosition();
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
    ASSERT(size_x > 0 && size_y > 0)(size_x)(size_y);
    return temp.GetSubBitmap(wxRect(mBitmapOffset.x,mBitmapOffset.y,size_x,size_y));
}

void Drag::draw(wxDC& dc) const
{
    if (!mActive)
    {
        return;
    }
    dc.DrawBitmap(mBitmap,mBitmapOffset + mPosition - mHotspot,true);
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
{
}

void Drag::DragInfo::reset()
{
    mOffset = 0;
    mMinOffset = 0;
    mMaxOffset = 0;

    // Determine boundaries
    std::set<model::TrackPtr> selectedTracks;
    BOOST_FOREACH( model::ClipPtr clip, getSelection().getClips() )
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
    mOffset = 
        std::min(mMaxOffset,
        std::max(mMinOffset, 
        indexOfTrackInTimeline - indexOfDraggedTrack));
}


model::TrackPtr Drag::DragInfo::getTrack(int index)
{
    return  mIsVideo ? getSequence()->getVideoTrack(index) : getSequence()->getAudioTrack(index);
}

int Drag::DragInfo::nTracks()
{
    return mIsVideo ? getSequence()->getVideoTracks().size() : getSequence()->getAudioTracks().size();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

model::TrackPtr Drag::trackOnTopOf(model::TrackPtr track)
{
    model::TrackPtr draggedTrack;
    VAR_DEBUG(track);
    DragInfo& info = getAssociatedInfo(track);
    int draggedTrackIndex = track->getIndex() - info.mOffset;
    return info.getTrack(draggedTrackIndex);
}

Drag::DragInfo& Drag::getAssociatedInfo(model::TrackPtr track)
{
    return track->isA<model::VideoTrack>() ? mVideo : mAudio;
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

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<< (std::ostream& os, const Drag& obj)
{
    os  << &obj                 << "|" 
        << obj.mSnap            << "|"
        << obj.mActive          << "|"
        << obj.mHotspot         << "|" 
        << obj.mPosition        << "|" 
        << obj.mVideo           << "|"
        << obj.mAudio           << "|"
        << obj.mDraggedTrack    << "|"
        << obj.mDropTrack;
    return os;
}

std::ostream& operator<< (std::ostream& os, const Drag::DragInfo& obj)
{
    os  << obj.mOffset    << "|"
        << obj.mMinOffset << "|"
        << obj.mMaxOffset;
    return os;
}

}} // namespace

