#include "AudioView.h"

#include <wx/dcmemory.h>
#include <boost/foreach.hpp>
#include "Layout.h"
#include "Track.h"
#include "TrackView.h"
#include "Sequence.h"
#include "PositionInfo.h"
#include "ViewMap.h"
#include "Divider.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

AudioView::AudioView(View* parent)
:   View(parent)
{
    VAR_DEBUG(this);

    // Not via onAudioTracksAdded: do not trigger a whole sequence of 
    // invalidateBitmaps calls: Bad performance and crashes
    // (view of second item added is not initialized when processing
    // the invalidateBitmap events for the first added item)
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getAudioTracks() )
    {
        new TrackView(track,this);
    }

    getSequence()->Bind(model::EVENT_ADD_AUDIO_TRACK,       &AudioView::onAudioTracksAdded,   this);
    getSequence()->Bind(model::EVENT_REMOVE_AUDIO_TRACK,    &AudioView::onAudioTracksRemoved,  this);
}

AudioView::~AudioView()
{
    VAR_DEBUG(this);

    getSequence()->Unbind(model::EVENT_ADD_AUDIO_TRACK,       &AudioView::onAudioTracksAdded,   this);
    getSequence()->Unbind(model::EVENT_REMOVE_AUDIO_TRACK,    &AudioView::onAudioTracksRemoved,  this);

    BOOST_FOREACH( model::TrackPtr track, getSequence()->getAudioTracks() )
    {
        delete getViewMap().getView(track);
    }
}

//////////////////////////////////////////////////////////////////////////
// MODEL EVENTS
//////////////////////////////////////////////////////////////////////////

/** @todo sequence::size event when adding clips, somewhere */

void AudioView::onAudioTracksAdded( model::EventAddAudioTracks& event )
{
    BOOST_FOREACH( model::TrackPtr track, event.getValue().addedTracks)
    {
        new TrackView(track,this);
    }
    invalidateBitmap();
    event.Skip();
}

void AudioView::onAudioTracksRemoved( model::EventRemoveAudioTracks& event )
{
    BOOST_FOREACH( model::TrackPtr track, event.getValue().removedTracks )
    {
        delete getViewMap().getView(track);
    }
    invalidateBitmap();
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

pixel AudioView::requiredWidth() const
{
    return getParent().requiredWidth();
}

pixel AudioView::requiredHeight() const
{
    pixel requiredHeight = 0;
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getAudioTracks() )
    {
        requiredHeight += track->getHeight() + Layout::sTrackDividerHeight;
    }
    return requiredHeight;
}

void AudioView::getPositionInfo(wxPoint position, PointerPositionInfo& info ) const
{
    int top = getDivider().getAudioPosition();
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getAudioTracks() )
    {
        int bottom = top + track->getHeight() + Layout::sTrackDividerHeight;
        if (position.y >= top && position.y < bottom)
        {
            info.track = track;
            info.trackPosition = top;
            info.onTrackDivider = (bottom - position.y <= Layout::sTrackDividerHeight);
            return;
        }
        top = bottom;
    }
}

void AudioView::draw(wxBitmap& bitmap) const
{
    wxMemoryDC dc(bitmap);
    int y = 0;
    dc.SetBrush(Layout::sTrackDividerBrush);
    dc.SetPen(Layout::sTrackDividerPen);
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getAudioTracks() )
    {
        dc.DrawBitmap(getViewMap().getView(track)->getBitmap(), 0, y, true);
        y += track->getHeight();
        dc.DrawRectangle(0, y, dc.GetSize().GetWidth(), y + Layout::sTrackDividerHeight);
        y += Layout::sTrackDividerHeight;
    }
}

}} // namespace
