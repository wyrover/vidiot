#include "Drag.h"

#include <wx/pen.h>
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
    ,   mVideoTrackOffset(0)
    ,   mAudioTrackOffset(0)
{
}

//////////////////////////////////////////////////////////////////////////
// START/STOP
//////////////////////////////////////////////////////////////////////////

void Drag::Start(wxPoint hotspot)
{
    mHotspot = hotspot;
    mPosition = hotspot;
    mVideoTrackOffset = 0;
    mBitmapOffset = wxPoint(0,0);

    mAudioTrackOffset = 0;
    PointerPositionInfo info = getMousePointer().getInfo(hotspot);
    mDraggedTrack = info.track;
    mDropTrack = info.track;
    VAR_DEBUG(*this);
    ASSERT(mDraggedTrack);

    mActive = true; // Must be done BEFORE getDragBitmap(), since it is used for creating that bitmap.
    BOOST_FOREACH( model::ClipPtr clip, getSelection().getClips() )
    {
        getViewMap().getView(clip)->invalidateBitmap();
    }
    mBitmap = getDragBitmap();
    MoveTo(hotspot);
}

void Drag::MoveTo(wxPoint position)
{
    VAR_DEBUG(*this);
    wxRegion redrawRegion;

    redrawRegion.Union(wxRect(mBitmapOffset + mPosition - mHotspot, mBitmap.GetSize())); // Redraw the old area (moved 'out' of this area)

    PointerPositionInfo info = getMousePointer().getInfo(position);
    if (!info.track || info.track == mDropTrack)
    {
        mHotspot.y -= mPosition.y - position.y; // Move the cursor without moving the dragged object (note: vertical only!)
    }
    else
    {
        if (info.track->isA<model::VideoTrack>())
        {
            mVideoTrackOffset = info.track->getIndex() -  mDraggedTrack->getIndex();
            mHotspot.y = position.y;
            mBitmap = getDragBitmap();
            ASSERT(mDraggedTrack->isA<model::VideoTrack>()); // Hopping over tracks not implemented 
        }
        else
        {
            mAudioTrackOffset = info.track->getIndex() -  mDraggedTrack->getIndex();
            mHotspot.y = position.y;
            mBitmap = getDragBitmap();
            ASSERT(mDraggedTrack->isA<model::AudioTrack>()); // Hopping over tracks not implemented 
        }
        mDropTrack = info.track;

        // todo: how to change 'mDraggedTrack' (via alt or via hopping over from video to audio, or by moving the pointer 'outside all tracks'
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
        // Determine which track is currently dragged above this track
        int draggedTrackIndex = track->getIndex() - mVideoTrackOffset;
        if ((draggedTrackIndex >= 0) && (draggedTrackIndex < getSequence()->getVideoTracks().size()))
        {
            VAR_DEBUG(draggedTrackIndex)(track->getIndex())(mVideoTrackOffset);
            // Draw the track that is currently dragged on top of this track
            model::TrackPtr draggedTrack = getSequence()->getVideoTrack(draggedTrackIndex);
            getViewMap().getView(draggedTrack)->drawForDragging(position,track->getHeight(),dc,dcMask);
        }
        // else: 
        // If this track is mapped to a dragged track beyond the range of tracks
        // then apparently, nothing needs to be drawn on top of this track.
        // Hence no 'else' handling here.
        position.y += track->getHeight();
    }

    // Draw audio tracks
    position.y = getDivider().getAudioPosition();
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getAudioTracks() )
    {
        // Determine which track is currently dragged above this track
        int draggedTrackIndex = track->getIndex() - mAudioTrackOffset;
        if ((draggedTrackIndex >= 0) && (draggedTrackIndex < getSequence()->getAudioTracks().size()))
        {
            VAR_DEBUG(draggedTrackIndex)(track->getIndex())(mAudioTrackOffset);
            // Draw the track that is currently dragged on top of this track
            model::TrackPtr draggedTrack = getSequence()->getAudioTrack(draggedTrackIndex);
            getViewMap().getView(draggedTrack)->drawForDragging(position,track->getHeight(),dc,dcMask);
        }
        // else: 
        // If this track is mapped to a dragged track beyond the range of tracks
        // then apparently, nothing needs to be drawn on top of this track.
        // Hence no 'else' handling here.
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
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<< (std::ostream& os, const Drag& obj)
{
    os  << &obj                     << "|" 
        << obj.mSnap                << "|"
        << obj.mActive              << "|"
        << obj.mHotspot             << "|" 
        << obj.mPosition            << "|" 
        << obj.mVideoTrackOffset    << "|"
        << obj.mAudioTrackOffset    << "|"
        << obj.mDraggedTrack        << "|"
        << obj.mDropTrack;
    return os;
}

}} // namespace

