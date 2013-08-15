// Copyright 2013 Eric Raijmakers.
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

#include "VideoView.h"

#include "Layout.h"
#include "Track.h"
#include "TrackView.h"
#include "Sequence.h"
#include "ViewMap.h"
#include "PositionInfo.h"
#include "Timeline.h"
#include "SequenceView.h"
#include "SequenceEvent.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

VideoView::VideoView(View* parent)
:   View(parent)
{
    VAR_DEBUG(this);

    // Not via onVideoTracksAdded: do not trigger a whole sequence of
    // invalidateBitmaps calls: Bad performance and crashes
    // (view of second item added is not initialized when processing
    // the invalidateBitmap events for the first added item)
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getVideoTracks() )
    {
        new TrackView(track,this);
    }

    getSequence()->Bind(model::EVENT_ADD_VIDEO_TRACK,       &VideoView::onVideoTracksAdded,    this);
    getSequence()->Bind(model::EVENT_REMOVE_VIDEO_TRACK,    &VideoView::onVideoTracksRemoved,  this);
}

VideoView::~VideoView()
{
    VAR_DEBUG(this);

    getSequence()->Unbind(model::EVENT_ADD_VIDEO_TRACK,       &VideoView::onVideoTracksAdded,    this);
    getSequence()->Unbind(model::EVENT_REMOVE_VIDEO_TRACK,    &VideoView::onVideoTracksRemoved,  this);

    BOOST_FOREACH( model::TrackPtr track, getSequence()->getVideoTracks() )
    {
        delete getViewMap().getView(track);
    }
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

// todo progress bar when initially opening a project.
// todo handle home+end+right+left(1 verder) + ctrlright+left (next cut...)

void VideoView::canvasResized()
{
    invalidateBitmap();
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getVideoTracks() )
    {
        getViewMap().getView(track)->canvasResized();
    }
}

wxSize VideoView::requiredSize() const
{
    int width = getSequenceView().minimumWidth();
    int height = 0;
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getVideoTracks() )
    {
        height += track->getHeight() + Layout::TrackDividerHeight;
    }
    return wxSize(width,height);
}

void VideoView::getPositionInfo(wxPoint position, PointerPositionInfo& info ) const
{
    int top = getSequenceView().getVideoPosition();
    BOOST_REVERSE_FOREACH( model::TrackPtr track, getSequence()->getVideoTracks() )
    {
        int bottom = top + track->getHeight() + Layout::TrackDividerHeight;
        if (position.y >= top && position.y < bottom)
        {
            info.track = track;
            info.trackPosition = top;
            info.onTrackDivider = (position.y - top <= Layout::TrackDividerHeight);
            getViewMap().getView(track)->getPositionInfo(position, info);
            return;
        }
        top = bottom;
    }
}

pixel VideoView::getPosition(model::TrackPtr track) const
{
    int y = 0;
    BOOST_REVERSE_FOREACH(model::TrackPtr _track, getSequence()->getVideoTracks())
    {
        y += Layout::TrackDividerHeight;
        if (track == _track)
        {
            break;
        }
        y += _track->getHeight();
    }
    return y;
}

//////////////////////////////////////////////////////////////////////////
// MODEL EVENTS
//////////////////////////////////////////////////////////////////////////

void VideoView::onVideoTracksAdded( model::EventAddVideoTracks& event )
{
    BOOST_FOREACH( model::TrackPtr track, event.getValue().addedTracks )
    {
        new TrackView(track,this);
    }
    invalidateBitmap();
    // Not via an event in sequence view, since the added video track must
    // first be incorporated in the VideoView (the divider height requires
    // the correct video height).
    getSequenceView().resetDividerPosition();
    event.Skip();
}

void VideoView::onVideoTracksRemoved( model::EventRemoveVideoTracks& event )
{
    BOOST_FOREACH( model::TrackPtr track, event.getValue().removedTracks )
    {
        delete getViewMap().getView(track);
    }
    invalidateBitmap();
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void VideoView::draw(wxBitmap& bitmap) const
{
    wxMemoryDC dc(bitmap);
    int y = 0;
    BOOST_REVERSE_FOREACH( model::TrackPtr track, getSequence()->getVideoTracks())
    {
        drawDivider(dc, y, Layout::TrackDividerHeight);
        y += Layout::TrackDividerHeight;
        dc.DrawBitmap(getViewMap().getView(track)->getBitmap(), wxPoint(0,y));
        y += track->getHeight();
    }
}

}} // namespace