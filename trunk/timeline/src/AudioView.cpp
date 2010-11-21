#include "AudioView.h"

#include <wx/dc.h>
#include <boost/foreach.hpp>
#include "Constants.h"
#include "UtilLog.h"
#include "Cursor.h"
#include "Track.h"
#include "TrackView.h"
#include "Zoom.h"
#include "Sequence.h"
#include "Timeline.h"
#include "MousePointer.h"
#include "ViewMap.h"

namespace gui { namespace timeline {

DEFINE_EVENT(AUDIO_UPDATE_EVENT, AudioUpdateEvent, AudioView*);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

AudioView::AudioView()
{
}

void AudioView::init()
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
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void AudioView::onAudioTrackUpdated( TrackUpdateEvent& event )
{
    LOG_INFO;
    getCursor().moveCursorOnUser(getCursor().getPosition()); // This is needed to reset iterators in model in case of clip addition/removal
    /** todo only redraw track */
    updateAudioBitmap();
}

//////////////////////////////////////////////////////////////////////////
// MODEL EVENTS
//////////////////////////////////////////////////////////////////////////

/** @todo sequence::size event when adding clips, somewhere */

void AudioView::onAudioTracksAdded( model::EventAddAudioTracks& event )
{
    BOOST_FOREACH( model::TrackPtr track, event.getValue().addedTracks)
    {
        TrackView* t = new TrackView(track);
        t->initTimeline(&getTimeline());
        t->Bind(TRACK_UPDATE_EVENT, &AudioView::onAudioTrackUpdated, this);
    }
    updateAudioSize();
}

void AudioView::onAudioTracksRemoved( model::EventRemoveAudioTracks& event )
{
    updateAudioSize();
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

const wxBitmap& AudioView::getAudio() const
{
    return mAudio;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

int AudioView::requiredWidth()
{
    return
        std::max(std::max(
        getZoom().timeToPixels(5 * Constants::sMinute),            // Minimum width of 5 minutes
        getZoom().ptsToPixels(getSequence()->getNumberOfFrames())),    // At least enough to hold all clips
        getTimeline().GetClientSize().GetWidth());                           // At least the widget size
}

int AudioView::requiredAudioHeight()
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

void AudioView::updateAudioSize()
{
    mAudio.Create(requiredWidth(),requiredAudioHeight());
    updateAudioBitmap();
}

void AudioView::updateAudioBitmap()
{
    wxMemoryDC dc(mAudio);
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
    QueueEvent(new AudioUpdateEvent(this));
}

}} // namespace
