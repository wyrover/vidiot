#include "Drag.h"

#include <wx/pen.h>
#include <wx/tooltip.h>
#include <boost/foreach.hpp>
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
    ,   mBitmap()
    ,   mActive(false)
    ,   mDraggedTrack()
    ,   mDropTrack()
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

    mVideo.mOffset = 0;
    mVideo.mMinOffset = 0;
    mVideo.mMaxOffset = 0;
    mAudio.mOffset = 0;
    mAudio.mMinOffset = 0;
    mAudio.mMaxOffset = 0;
    VAR_DEBUG(*this);

    ASSERT(mDraggedTrack);

    mActive = true; // Must be done BEFORE getDragBitmap(), since it is used for creating that bitmap.

    // Determine the minimal allowed offsets
    std::set<model::TrackPtr> selectedTracks;
    BOOST_FOREACH( model::ClipPtr clip, getSelection().getClips() )
    {
        selectedTracks.insert(clip->getTrack());
    }
    BOOST_FOREACH( model::TrackPtr track, selectedTracks )
    {
        if (track->isA<model::VideoTrack>())
        {
            mVideo.mMinOffset = std::min(mVideo.mMinOffset, track->getIndex() * -1);
        }
        else
        {
            mAudio.mMinOffset = std::min(mAudio.mMinOffset, track->getIndex() * -1);
        }
    }

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
        if (info.track)
        {
            model::TrackPtr newDraggedTrack = trackOnTopOf(info.track);
            if (newDraggedTrack)
            {
                mDraggedTrack = newDraggedTrack;
            }
        }
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

        bool toVideo = info.track->isA<model::VideoTrack>();
        bool fromVideo = mDraggedTrack->isA<model::VideoTrack>();

        if (toVideo == fromVideo)
        {
            // The pointer moved between video tracks or between audio tracks.
            if (toVideo)
            {
                mVideo.mOffset = std::max(mAudio.mMinOffset, info.track->getIndex() -  mDraggedTrack->getIndex());
            }
            else
            {
                mAudio.mOffset = std::max(mAudio.mMinOffset, info.track->getIndex() -  mDraggedTrack->getIndex());
            }
        }
        else
        {
            // Pointer moved from video to audio. The 'dragged track' must be updated.
            // The offsets are not changed. These are only changed when moving a track
            // of type x to another track of type x.
            if (info.track)
            {
                model::TrackPtr newDraggedTrack = trackOnTopOf(info.track);
                if (newDraggedTrack)
                {
                    mDraggedTrack = newDraggedTrack;
                }
            }
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

wxRect Drag::GetImageRect(const wxPoint& pos) const
{
    return wxRect(pos.x,pos.y,mBitmap.GetWidth(),mBitmap.GetHeight());
}

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

bool Drag::isActive() const
{
    return mActive;
}

//////////////////////////////////////////////////////////////////////////
// DRAW
//////////////////////////////////////////////////////////////////////////

void Drag::draw(wxDC& dc) const
{
    if (!mActive)
    {
        return;
    }
    dc.DrawBitmap(mBitmap,mBitmapOffset + mPosition - mHotspot,true);
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

model::TrackPtr Drag::trackOnTopOf(model::TrackPtr track)
{
    model::TrackPtr draggedTrack;
    VAR_DEBUG(track);

    if (track->isA<model::VideoTrack>())
    {
        int draggedTrackIndex = track->getIndex() - mVideo.mOffset;
        int nTracks = getSequence()->getVideoTracks().size();
        if ((draggedTrackIndex >= 0) && (draggedTrackIndex < nTracks))
        {
            return getSequence()->getVideoTrack(draggedTrackIndex);
        }
    }
    else
    {
        int draggedTrackIndex = track->getIndex() - mAudio.mOffset;
        int nTracks = getSequence()->getAudioTracks().size();
        if ((draggedTrackIndex >= 0) && (draggedTrackIndex < nTracks))
        {
            return getSequence()->getAudioTrack(draggedTrackIndex);
        }
    }
    return model::TrackPtr();
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<< (std::ostream& os, const Drag& obj)
{
    os  << &obj                     << "|" 
        << obj.mSnap                << "|"
        << obj.mActive              << "|"
        << obj.mHotspot             << "|" 
        << obj.mPosition            << "|" 
        << obj.mVideo.mOffset    << "|"
        << obj.mAudio.mOffset    << "|"
        << obj.mDraggedTrack        << "|"
        << obj.mDropTrack;
    return os;
}

}} // namespace

