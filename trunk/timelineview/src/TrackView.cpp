// Copyright 2013,2014 Eric Raijmakers.
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

#include "TrackView.h"

#include "Node.h"
#include "Clip.h"
#include "ClipView.h"
#include "Drag.h"
#include "Drag_Shift.h"
#include "Layout.h"
#include "PositionInfo.h"
#include "Selection.h"
#include "Track.h"
#include "TrackEvent.h"
#include "Transition.h"
#include "UtilLogWxwidgets.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "VideoTrack.h"
#include "ViewMap.h"
#include "Zoom.h"

namespace gui { namespace timeline {
//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

TrackView::TrackView(const model::TrackPtr& track, View* parent)
:   View(parent)
,   mTrack(track)
,   mY(boost::none)
,   mClips()
{
    VAR_DEBUG(this);
    ASSERT(mTrack); // Must be initialized

    getViewMap().registerView(mTrack,this);

    // Not via onClipsAdded: do not trigger a whole sequence of
    // invalidateBitmaps calls: Bad performance and crashes
    // (view of second item added is not initialized when processing
    // the invalidateBitmap events for the first added item)
    for ( model::IClipPtr clip : mTrack->getClips() )
    {
        new ClipView(clip,this);
    }

    mTrack->Bind(model::EVENT_ADD_CLIPS,        &TrackView::onClipsAdded,       this);
    mTrack->Bind(model::EVENT_REMOVE_CLIPS,     &TrackView::onClipsRemoved,     this);
}

TrackView::~TrackView()
{
    VAR_DEBUG(this);

    mTrack->Unbind(model::EVENT_ADD_CLIPS,      &TrackView::onClipsAdded,       this);
    mTrack->Unbind(model::EVENT_REMOVE_CLIPS,   &TrackView::onClipsRemoved,     this);

    getViewMap().unregisterView(mTrack);

    for ( model::IClipPtr clip : mTrack->getClips() )
    {
        delete getViewMap().getView(clip);
    }
}

model::TrackPtr TrackView::getTrack() const
{
    return mTrack;
}

//////////////////////////////////////////////////////////////////////////
// VIEW
//////////////////////////////////////////////////////////////////////////

pixel TrackView::getX() const
{
    return getParent().getX();
}

pixel TrackView::getY() const
{
    if (!mY)
    {
        if (mTrack->isA<model::VideoTrack>())
        {
            int y = getSequence()->getDividerPosition();
            for (model::TrackPtr track : getSequence()->getVideoTracks() )
            {
                y -= track->getHeight();
                if (track == mTrack)
                {
                    break;
                }
                y -= Layout::TrackDividerHeight;
            }
            mY.reset(y);
        }
        else
        {
            int y = getSequenceView().getAudio().getY();
            for (model::TrackPtr track : getSequence()->getAudioTracks())
            {
                if (track == mTrack)
                {
                    break;
                }
                y += track->getHeight() + Layout::TrackDividerHeight;
            }
            mY.reset(y);
        }
    }
    return *mY;
}

pixel TrackView::getW() const
{
    return getParent().getW();
}

pixel TrackView::getH() const
{
    return mTrack->getHeight();
}

void TrackView::invalidateRect()
{
    mY.reset();
    mClips.reset();
}

void TrackView::draw(wxDC& dc, const wxRegion& region, const wxPoint& offset) const
{
    if (!mClips)
    {
        mClips = boost::make_shared<ClipLookupMap>();
        for ( model::IClipPtr clip : mTrack->getClips() )
        {
            ClipView* view = getViewMap().getView(clip);
            mClips->insert( std::make_pair(view->getLeftPixel(), clip) );
            // Not called during invalidateRect (simply does list reset), therefore must be done here.
            // Furthermore, if the track rect was invalidated by moving clips (i.s.o. adding/removing clips)
            // the moved clips must have their rect invalidated. Typical use case: drag clip to end of track.
            view->invalidateRect();
        }
    }

    // Clear region exposed by shift. Is done before drawing clips.
    // Note that everything to the right of the shift is cleared.
    // This was done since sometimes part of the region to the right
    // of the shift position was not cleared properly.
    Shift shift = getDrag().getShift();
    if (shift)
    {
        wxRect toBeCleared(shift->getPixelPosition(),getY(),getTimeline().GetVirtualSize().GetWidth() - shift->getPixelPosition(),getH());
        getTimeline().clearRect(dc,region,offset,toBeCleared);
    }

    // Determine which clips are 'in the region'
    pixel redrawFromPixel = region.GetBox().GetLeft() + offset.x;
    std::map<pixel, model::IClipPtr>::const_iterator from = mClips->upper_bound(redrawFromPixel);
    model::IClipPtr fromClip = (from != mClips->end()) ? from->second : model::IClipPtr();
    while (fromClip &&
        fromClip->getPrev() &&
        getViewMap().getView(fromClip)->getRightPixel() >= redrawFromPixel)
    {
        // See definition of upper_bound (returns first clip 'after' redrawFromPixel). Hence, we need one clip earlier.
        // Furthermore, if there's an in-transition at redrawFromPixel, it must be redrawn also.
        fromClip = fromClip->getPrev();
    }
    if (!fromClip && !mTrack->getClips().empty())
    {
        // Ensure that the first clip is not '0'. Otherwise, nothing is redrawn.
        // Required for 1-clip length tracks.
        fromClip = mTrack->getClips().front();
    }

    pixel redrawToPixel = region.GetBox().GetRight() + offset.x;
    std::map<pixel, model::IClipPtr>::const_iterator to = mClips->upper_bound(redrawToPixel);
    model::IClipPtr toClip = (to != mClips->end()) ? to->second : model::IClipPtr();
    while (toClip && getViewMap().getView(toClip)->getLeftPixel() <= redrawToPixel)
    {
        // If there's an out-transition at the position, it must be redrawn also.
        toClip = toClip->getNext();
    }

    // Draw the clips that are 'in the region' - first clips, then transitions on top.
    model::IClips transitions;
    model::IClipPtr clip = fromClip;
    while (clip != toClip)
    {
        if (clip->isA<model::Transition>())
        {
            transitions.push_back(clip); // Handle later (shown on top of clips)
        }
        else
        {
            getViewMap().getView(clip)->draw(dc, region, offset);
        }
        clip = clip->getNext();
    }

    for ( model::IClipPtr transition : transitions )
    {
        getViewMap().getView(transition)->draw(dc, region, offset);
    }

    // Clear region to the right of clips
    const model::IClips& clips = mTrack->getClips();
    pixel right = getTimeline().getShift(); // When shift trimming, move the cleared region also
    if (!clips.empty())
    {
        right += getViewMap().getView(clips.back())->getRightPixel();
    }
    if (shift && shift->getPixelPosition() <= right) // if shift is beyond the timeline length, still the length of the shift != 0
    {
       right += shift->getPixelLength(); // When shift dragging near the end of the timeline, the track can become longer
    }
    getTimeline().clearRect(dc,region,offset,wxRect(right, getY(), getTimeline().GetVirtualSize().GetWidth() - right, getH()));
}

//////////////////////////////////////////////////////////////////////////
// MODEL EVENTS
//////////////////////////////////////////////////////////////////////////

void TrackView::onClipsAdded(model::EventAddClips& event )
{
    for ( model::IClipPtr clip : event.getValue().addClips )
    {
        new ClipView(clip,this);
    }
    invalidateRect();
    getTimeline().Refresh(false);
    event.Skip();
}

void TrackView::onClipsRemoved(model::EventRemoveClips& event )
{
    for ( model::IClipPtr clip : event.getValue().removeClips )
    {
        delete getViewMap().getView(clip);
    }
    invalidateRect();
    getTimeline().Refresh(false);
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// DRAWING EVENTS
//////////////////////////////////////////////////////////////////////////

void TrackView::getPositionInfo(const wxPoint& position, PointerPositionInfo& info) const
{
    wxPoint adjustedPosition(position);

    Shift shift = getDrag().getShift();
    if (shift && (position.x >= shift->getPixelPosition()))
    {
        // Apply shift if (A) shift is enabled, and (B) current position is after the shift start
        if (position.x >= shift->getPixelPosition() + shift->getPixelLength())
        {
            // Clip is AFTER the shifted area, adjust accordingly
            adjustedPosition.x -= shift->getPixelLength();
        }
        else
        {
            return; // Inside shifted area. No clip there.
        }
    }

    info.clip = mTrack->getClip(getZoom().pixelsToPts(adjustedPosition.x));
    if (info.clip)
    {
        getViewMap().getView(info.clip)->getPositionInfo(adjustedPosition, info);
    }
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void TrackView::drawForDragging(const wxPoint& position, int height, wxDC& dc, wxDC& dcMask) const
{
    model::IClips transitions;
    for ( model::IClipPtr clip : mTrack->getClips() )
    {
        if (clip->isA<model::Transition>())
        {
            transitions.push_back(clip); // Handle later (are shown on top of clips)
            continue;
        }
        ClipView* view = getViewMap().getView(clip);
        pixel left = view->getLeftPixel();
        view->drawForDragging(wxPoint(position.x + left,position.y), height, dc, dcMask);
    }
    for ( model::IClipPtr transition : transitions )
    {
        ClipView* view = getViewMap().getView(transition);
        pixel left = view->getLeftPixel();
        view->drawForDragging(wxPoint(position.x + left,position.y), height, dc, dcMask);
    }
}

}} // namespace
