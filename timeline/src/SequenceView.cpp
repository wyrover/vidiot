#include "SequenceView.h"

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

DEFINE_EVENT(VIDEO_UPDATE_EVENT, VideoUpdateEvent, SequenceView*);
DEFINE_EVENT(AUDIO_UPDATE_EVENT, AudioUpdateEvent, SequenceView*);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

SequenceView::SequenceView()
{
}

void SequenceView::init()
{

    model::TrackChange videoTracks(getSequence()->getVideoTracks());
    onVideoTracksAdded(model::EventAddVideoTracks(videoTracks));

    model::TrackChange audioTracks(getSequence()->getAudioTracks());
    onAudioTracksAdded(model::EventAddAudioTracks(audioTracks));

    getSequence()->Bind(model::EVENT_ADD_VIDEO_TRACK,       &SequenceView::onVideoTracksAdded,    this);
    getSequence()->Bind(model::EVENT_REMOVE_VIDEO_TRACK,    &SequenceView::onVideoTracksRemoved,  this);
    getSequence()->Bind(model::EVENT_ADD_AUDIO_TRACK,       &SequenceView::onAudioTracksAdded,   this);
    getSequence()->Bind(model::EVENT_REMOVE_AUDIO_TRACK,    &SequenceView::onAudioTracksRemoved,  this);
}

SequenceView::~SequenceView()
{
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void SequenceView::onVideoTrackUpdated( TrackUpdateEvent& event )
{
    LOG_INFO;
    getCursor().moveCursorOnUser(getCursor().getPosition()); // This is needed to reset iterators in model in case of clip addition/removal
    /** todo only redraw track */
    updateVideoBitmap();
}

void SequenceView::onAudioTrackUpdated( TrackUpdateEvent& event )
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

void SequenceView::onVideoTracksAdded( model::EventAddVideoTracks& event )
{
    BOOST_FOREACH( model::TrackPtr track, event.getValue().addedTracks)
    {
        TrackView* t = new TrackView(track);
        t->initTimeline(&getTimeline());
        t->Bind(TRACK_UPDATE_EVENT, &SequenceView::onVideoTrackUpdated, this);
    }
    updateVideoSize();
}

void SequenceView::onVideoTracksRemoved( model::EventRemoveVideoTracks& event )
{
    updateVideoSize();
}

void SequenceView::onAudioTracksAdded( model::EventAddAudioTracks& event )
{
    BOOST_FOREACH( model::TrackPtr track, event.getValue().addedTracks)
    {
        TrackView* t = new TrackView(track);
        t->initTimeline(&getTimeline());
        t->Bind(TRACK_UPDATE_EVENT, &SequenceView::onAudioTrackUpdated, this);
    }
    updateAudioSize();
}

void SequenceView::onAudioTracksRemoved( model::EventRemoveAudioTracks& event )
{
    updateAudioSize();
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

const wxBitmap& SequenceView::getVideo() const
{
    return mVideo;
}

const wxBitmap& SequenceView::getAudio() const
{
    return mAudio;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

int SequenceView::requiredWidth()
{
    return
        std::max(std::max(
        getZoom().timeToPixels(5 * Constants::sMinute),            // Minimum width of 5 minutes
        getZoom().ptsToPixels(getSequence()->getNumberOfFrames())),    // At least enough to hold all clips
        getTimeline().GetClientSize().GetWidth());                           // At least the widget size
}

int SequenceView::requiredVideoHeight()
{
    int requiredHeight = 0;
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getVideoTracks() )
    {
        requiredHeight += track->getHeight() + Constants::sTrackDividerHeight;
    }
    return requiredHeight;
}

int SequenceView::requiredAudioHeight()
{
    int requiredHeight = 0;
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getVideoTracks() )
    {
        requiredHeight += track->getHeight() + Constants::sTrackDividerHeight;
    }
    return requiredHeight;
}

void SequenceView::getPositionInfo(wxPoint position, PointerPositionInfo& info )
{
    int top = getTimeline().getDividerPosition() - getVideo().GetHeight();
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getVideoTracks() )
    {
        int bottom = top + track->getHeight() + Constants::sTrackDividerHeight;
        if (position.y >= top && position.y < bottom)
        {
            info.track = track;
            info.trackPosition = top;
            info.onTrackDivider = (position.y - top <= Constants::sTrackDividerHeight);
            return;
        }
        top = bottom;
    }
    // Find possible audiotrack under pointer
    top = getTimeline().getDividerPosition() + Constants::sAudioVideoDividerHeight;
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

void SequenceView::updateVideoSize()
{
    mVideo.Create(requiredWidth(),requiredVideoHeight());
    updateVideoBitmap();
}

void SequenceView::updateAudioSize()
{
    mAudio.Create(requiredWidth(),requiredAudioHeight());
    updateAudioBitmap();
}

void SequenceView::updateVideoBitmap()
{
    wxMemoryDC dc(mVideo);
    int y = 0;
    BOOST_REVERSE_FOREACH( model::TrackPtr track, getSequence()->getVideoTracks())
    {
        dc.SetBrush(Constants::sTrackDividerBrush);
        dc.SetPen(Constants::sTrackDividerPen);
        dc.DrawRectangle(0,y,mVideo.GetWidth(),y + Constants::sTrackDividerHeight);
        y += Constants::sTrackDividerHeight;
        wxBitmap b = getViewMap().getView(track)->getBitmap();
        dc.DrawBitmap(b,wxPoint(0,y));
        y += track->getHeight();
    }
    QueueEvent(new VideoUpdateEvent(this));
}

void SequenceView::updateAudioBitmap()
{
    wxMemoryDC dc(mAudio);
    int y = 0;

    BOOST_FOREACH( model::TrackPtr track, getSequence()->getAudioTracks() )
    {
        wxBitmap b = getViewMap().getView(track)->getBitmap();
        dc.DrawBitmap(b,wxPoint(0,y));
        y += track->getHeight();
        dc.SetBrush(Constants::sTrackDividerBrush);
        dc.SetPen(Constants::sTrackDividerPen);
        dc.DrawRectangle(0,y,mVideo.GetWidth(),y+Constants::sTrackDividerHeight);
        y += Constants::sTrackDividerHeight;
    }
    QueueEvent(new AudioUpdateEvent(this));
}

}} // namespace
