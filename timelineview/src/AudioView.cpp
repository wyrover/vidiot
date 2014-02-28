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

#include "AudioView.h"

#include "DividerView.h"
#include "Layout.h"
#include "PositionInfo.h"
#include "Sequence.h"
#include "SequenceEvent.h"
#include "SequenceView.h"
#include "Timeline.h"
#include "Track.h"
#include "TrackEvent.h"
#include "TrackView.h"
#include "ViewMap.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

AudioView::AudioView(View* parent)
:   View(parent)
,   mHeight(boost::none)
{
    VAR_DEBUG(this);

    // Not via onAudioTracksAdded: do not trigger a whole sequence of
    // invalidateBitmaps calls: Bad performance and crashes
    // (view of second item added is not initialized when processing
    // the invalidateBitmap events for the first added item)
    for ( model::TrackPtr track : getSequence()->getAudioTracks() )
    {
        new TrackView(track, this);
        new DividerView(this, Layout::TrackDividerHeight, track);
        track->Bind(model::EVENT_HEIGHT_CHANGED, &AudioView::onTrackHeightChanged, this);

    }

    getSequence()->Bind(model::EVENT_ADD_AUDIO_TRACK,       &AudioView::onAudioTracksAdded,   this);
    getSequence()->Bind(model::EVENT_REMOVE_AUDIO_TRACK,    &AudioView::onAudioTracksRemoved,  this);
}

AudioView::~AudioView()
{
    VAR_DEBUG(this);

    getSequence()->Unbind(model::EVENT_ADD_AUDIO_TRACK,       &AudioView::onAudioTracksAdded,   this);
    getSequence()->Unbind(model::EVENT_REMOVE_AUDIO_TRACK,    &AudioView::onAudioTracksRemoved,  this);

    for ( model::TrackPtr track : getSequence()->getAudioTracks() )
    {
        delete getViewMap().getView(track);
        delete getViewMap().getDivider(track);
        track->Unbind(model::EVENT_HEIGHT_CHANGED, &AudioView::onTrackHeightChanged, this);
    }
}

//////////////////////////////////////////////////////////////////////////
// VIEW
//////////////////////////////////////////////////////////////////////////

pixel AudioView::getX() const
{
    return getParent().getX();
}

pixel AudioView::getY() const
{
    return getSequence()->getDividerPosition() + Layout::AudioVideoDividerHeight;
}

pixel AudioView::getW() const
{
    return getParent().getW();
}

pixel AudioView::getH() const
{
    if (!mHeight)
    {
        int height = 0;
        for ( model::TrackPtr track : getSequence()->getAudioTracks() )
        {
            height += track->getHeight() + Layout::TrackDividerHeight;
        }
        mHeight.reset(height);
    }
    return *mHeight;
}

void AudioView::invalidateRect()
{
    mHeight.reset();
    for ( model::TrackPtr track : getSequence()->getAudioTracks() )
    {
        getViewMap().getView(track)->invalidateRect();
        getViewMap().getDivider(track)->invalidateRect();
    }
}

void AudioView::draw(wxDC& dc, const wxRegion& region, const wxPoint& offset) const
{
    for ( model::TrackPtr track : getSequence()->getAudioTracks() )
    {
        getViewMap().getView(track)->draw(dc,region,offset);
        getViewMap().getDivider(track)->draw(dc,region,offset);
    }
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

void AudioView::getPositionInfo(const wxPoint& position, PointerPositionInfo& info ) const
{
    int top = getY();
    for ( model::TrackPtr track : getSequence()->getAudioTracks() )
    {
        int bottom = top + track->getHeight() + Layout::TrackDividerHeight;
        if (position.y >= top && position.y < bottom)
        {
            info.track = track;
            info.trackPosition = top;
            info.onTrackDivider = (bottom - position.y <= Layout::TrackDividerHeight);
            getViewMap().getView(track)->getPositionInfo(position, info);
            return;
        }
        top = bottom;
    }
}

//////////////////////////////////////////////////////////////////////////
// MODEL EVENTS
//////////////////////////////////////////////////////////////////////////

void AudioView::onAudioTracksAdded(model::EventAddAudioTracks& event )
{
    for ( model::TrackPtr track : event.getValue().addedTracks)
    {
        new TrackView(track,this);
        new DividerView(this, Layout::TrackDividerHeight, track);
        track->Bind(model::EVENT_HEIGHT_CHANGED, &AudioView::onTrackHeightChanged, this);
    }
    getParent().invalidateRect(); // Will cause this::invalidateRect()
    getTimeline().Refresh(false);
    event.Skip();
}

void AudioView::onAudioTracksRemoved(model::EventRemoveAudioTracks& event )
{
    for ( model::TrackPtr track : event.getValue().removedTracks )
    {
        delete getViewMap().getView(track);
        delete getViewMap().getDivider(track);
        track->Unbind(model::EVENT_HEIGHT_CHANGED, &AudioView::onTrackHeightChanged, this);
    }
    getParent().invalidateRect(); // Will cause this::invalidateRect()
    getTimeline().Refresh(false);
    event.Skip();
}

void AudioView::onTrackHeightChanged(model::EventHeightChanged& event )
{
    getParent().invalidateRect(); // Will cause this::invalidateRect()
    getTimeline().Refresh(false);
    event.Skip();
}

}} // namespace