#include "AudioView.h"

#include "Layout.h"
#include "Track.h"
#include "TrackView.h"
#include "Sequence.h"
#include "PositionInfo.h"
#include "ViewMap.h"
#include "SequenceView.h"
#include "SequenceEvent.h"

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
// GET/SET
//////////////////////////////////////////////////////////////////////////

wxSize AudioView::requiredSize() const
{
    int width = getSequenceView().minimumWidth();
    int height = 0;
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getAudioTracks() )
    {
        height += track->getHeight() + Layout::TrackDividerHeight;
    }
    return wxSize(width,height);
}

void AudioView::getPositionInfo(wxPoint position, PointerPositionInfo& info ) const
{
    int top = getSequenceView().getAudioPosition();
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getAudioTracks() )
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

pixel AudioView::getPosition(model::TrackPtr track) const
{
    int y = 0;
    BOOST_FOREACH(model::TrackPtr _track, getSequence()->getAudioTracks())
    {
        if (track == _track)
        {
            break;
        }
        y += _track->getHeight();
        y += Layout::TrackDividerHeight;
    }
    return y;
}

//////////////////////////////////////////////////////////////////////////
// MODEL EVENTS
//////////////////////////////////////////////////////////////////////////

void AudioView::onAudioTracksAdded( model::EventAddAudioTracks& event )
{
    BOOST_FOREACH( model::TrackPtr track, event.getValue().addedTracks)
    {
        new TrackView(track,this);
    }
    invalidateBitmap();
    // Not via an event in sequence view, since the added audio track must
    // first be incorporated in the AudioView (the divider height requires
    // the correct audio height).
    getSequenceView().resetDividerPosition();
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
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void AudioView::draw(wxBitmap& bitmap) const
{
    wxMemoryDC dc(bitmap);
    int y = 0;
    dc.SetBrush(Layout::get().TrackDividerBrush);
    dc.SetPen(Layout::get().TrackDividerPen);
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getAudioTracks() )
    {
        dc.DrawBitmap(getViewMap().getView(track)->getBitmap(), wxPoint(0,y));
        y += track->getHeight();
        dc.DrawRectangle(0, y, dc.GetSize().GetWidth(), Layout::TrackDividerHeight);
        y += Layout::TrackDividerHeight;
    }
}

}} // namespace