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

#include "AudioView.h"

#include "Layout.h"
#include "Track.h"
#include "TrackView.h"
#include "Sequence.h"
#include "ViewMap.h"
#include "PositionInfo.h"
#include "SequenceView.h"
#include "SequenceEvent.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

VideoView::VideoView(View* parent)
:   View(parent)
,   mHeight(boost::none)
{
    VAR_DEBUG(this);

    // Not via onVideoTracksAdded: do not trigger a whole sequence of
    // invalidateBitmaps calls: Bad performance and crashes
    // (view of second item added is not initialized when processing
    // the invalidateBitmap events for the first added item)
    for ( model::TrackPtr track : getSequence()->getVideoTracks() )
    {
        new TrackView(track,this);
        new DividerView(this, Layout::TrackDividerHeight, track);
        track->Bind(model::EVENT_HEIGHT_CHANGED, &VideoView::onTrackHeightChanged, this);
    }

    getSequence()->Bind(model::EVENT_ADD_VIDEO_TRACK,       &VideoView::onVideoTracksAdded,    this);
    getSequence()->Bind(model::EVENT_REMOVE_VIDEO_TRACK,    &VideoView::onVideoTracksRemoved,  this);
}

VideoView::~VideoView()
{
    VAR_DEBUG(this);

    getSequence()->Unbind(model::EVENT_ADD_VIDEO_TRACK,       &VideoView::onVideoTracksAdded,    this);
    getSequence()->Unbind(model::EVENT_REMOVE_VIDEO_TRACK,    &VideoView::onVideoTracksRemoved,  this);

    for ( model::TrackPtr track : getSequence()->getVideoTracks() )
    {
        delete getViewMap().getView(track);
        delete getViewMap().getDivider(track);
        track->Unbind(model::EVENT_HEIGHT_CHANGED, &VideoView::onTrackHeightChanged, this);
    }
}

//////////////////////////////////////////////////////////////////////////
// VIEW
//////////////////////////////////////////////////////////////////////////

pixel VideoView::getX() const
{
    return getParent().getX();
}

pixel VideoView::getY() const
{
    return getSequence()->getDividerPosition() - getH();
}

pixel VideoView::getW() const
{
    return getParent().getW();
}

pixel VideoView::getH() const
{
    if (!mHeight)
    {
        int height = 0;
        for ( model::TrackPtr track : getSequence()->getVideoTracks() )
        {
            height += track->getHeight() + Layout::TrackDividerHeight;
        }
        mHeight.reset(height);
    }
    return *mHeight;
}

void VideoView::invalidateRect()
{
    mHeight.reset();
    for ( model::TrackPtr track : getSequence()->getVideoTracks() )
    {
        getViewMap().getView(track)->invalidateRect();
        getViewMap().getDivider(track)->invalidateRect();
    }
}

void VideoView::draw(wxDC& dc, const wxRegion& region, const wxPoint& offset) const
{
    for ( model::TrackPtr track : getSequence()->getVideoTracks() )
    {
        getViewMap().getView(track)->draw(dc,region,offset);
        getViewMap().getDivider(track)->draw(dc,region,offset);
    }
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

void VideoView::getPositionInfo(const wxPoint& position, PointerPositionInfo& info ) const
{
    int y = getSequence()->getDividerPosition();
    for ( model::TrackPtr track : getSequence()->getVideoTracks() )
    {
        int top = y - (track->getHeight()  + Layout::TrackDividerHeight);
        if (position.y >= top && position.y < y)
        {
            info.track = track;
            info.trackPosition = top;
            info.onTrackDivider = (position.y - top <= Layout::TrackDividerHeight);
            getViewMap().getView(track)->getPositionInfo(position, info);
            return;
        }
        y = top;
    }
}

//////////////////////////////////////////////////////////////////////////
// MODEL EVENTS
//////////////////////////////////////////////////////////////////////////

void VideoView::onVideoTracksAdded(model::EventAddVideoTracks& event )
{
    for ( model::TrackPtr track : event.getValue().addedTracks )
    {
        new TrackView(track,this);
        new DividerView(this, Layout::TrackDividerHeight, track);
        track->Bind(model::EVENT_HEIGHT_CHANGED, &VideoView::onTrackHeightChanged, this);
    }
    getSequenceView().resetDividerPosition(); // Will cause this::invalidateRect() and Timeline::Refresh()
    event.Skip();
}

void VideoView::onVideoTracksRemoved(model::EventRemoveVideoTracks& event )
{
    for ( model::TrackPtr track : event.getValue().removedTracks )
    {
        delete getViewMap().getView(track);
        delete getViewMap().getDivider(track);
        track->Unbind(model::EVENT_HEIGHT_CHANGED, &VideoView::onTrackHeightChanged, this);
    }
    getSequenceView().resetDividerPosition(); // Will cause this::invalidateRect() and Timeline::Refresh()
    event.Skip();
}

void VideoView::onTrackHeightChanged(model::EventHeightChanged& event )
{
    getParent().invalidateRect(); // Will cause this::invalidateRect()
    getTimeline().Refresh(false);
    event.Skip();
}

}} // namespace