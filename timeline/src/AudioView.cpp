#include "AudioView.h"

#include <wx/dcmemory.h>
#include <boost/foreach.hpp>
#include "Constants.h"
#include "Track.h"
#include "TrackView.h"
#include "Sequence.h"
#include "MousePointer.h"
#include "ViewMap.h"
#include "MousePointer.h"
#include "Timeline.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

AudioView::AudioView(View* parent)
:   View(parent)
{
    model::TrackChange audioTracks(getSequence()->getAudioTracks());
    onAudioTracksAdded(model::EventAddAudioTracks(audioTracks));

    getSequence()->Bind(model::EVENT_ADD_AUDIO_TRACK,       &AudioView::onAudioTracksAdded,   this);
    getSequence()->Bind(model::EVENT_REMOVE_AUDIO_TRACK,    &AudioView::onAudioTracksRemoved,  this);
}

AudioView::~AudioView()
{
}

//////////////////////////////////////////////////////////////////////////
// MODEL EVENTS
//////////////////////////////////////////////////////////////////////////

/** @todo sequence::size event when adding clips, somewhere */

void AudioView::onAudioTracksAdded( model::EventAddAudioTracks& event )
{
    BOOST_FOREACH( model::TrackPtr track, event.getValue().addedTracks)
    {
        TrackView* t = new TrackView(track,this);
    }
    invalidateBitmap();
}

void AudioView::onAudioTracksRemoved( model::EventRemoveAudioTracks& event )
{
    invalidateBitmap();
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

int AudioView::requiredWidth()
{
    return getParent().requiredWidth();
}

int AudioView::requiredHeight()
{
    int requiredHeight = 0;
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getAudioTracks() )
    {
        requiredHeight += track->getHeight() + Constants::sTrackDividerHeight;
    }
    return requiredHeight;
}

void AudioView::getPositionInfo(wxPoint position, PointerPositionInfo& info )
{
    int top = getTimeline().getDividerPosition() + Constants::sAudioVideoDividerHeight;
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getAudioTracks() )
    {
        int bottom = top + track->getHeight() + Constants::sTrackDividerHeight;
        if (position.y >= top && position.y < bottom)
        {
            info.track = track;
            info.trackPosition = top;
            info.onTrackDivider = (bottom - position.y <= Constants::sTrackDividerHeight);
            return;
        }
        top = bottom;
    }
}

void AudioView::draw(wxBitmap& bitmap)
{
    wxMemoryDC dc(bitmap);
    int y = 0;
    dc.SetBrush(Constants::sTrackDividerBrush);
    dc.SetPen(Constants::sTrackDividerPen);
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getAudioTracks() )
    {
        dc.DrawBitmap(getViewMap().getView(track)->getBitmap(), 0, y, true);
        y += track->getHeight();
        dc.DrawRectangle(0, y, dc.GetSize().GetWidth(), y + Constants::sTrackDividerHeight);
        y += Constants::sTrackDividerHeight;
    }
}

}} // namespace
