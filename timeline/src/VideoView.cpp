#include "VideoView.h"

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

DEFINE_EVENT(VIDEO_UPDATE_EVENT, VideoUpdateEvent, VideoView*);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

VideoView::VideoView()
{
}

void VideoView::init()
{

    model::TrackChange videoTracks(getSequence()->getVideoTracks());
    onVideoTracksAdded(model::EventAddVideoTracks(videoTracks));

    getSequence()->Bind(model::EVENT_ADD_VIDEO_TRACK,       &VideoView::onVideoTracksAdded,    this);
    getSequence()->Bind(model::EVENT_REMOVE_VIDEO_TRACK,    &VideoView::onVideoTracksRemoved,  this);
}

VideoView::~VideoView()
{
}

//////////////////////////////////////////////////////////////////////////
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void VideoView::onVideoTrackUpdated( TrackUpdateEvent& event )
{
    LOG_INFO;
    getCursor().moveCursorOnUser(getCursor().getPosition()); // This is needed to reset iterators in model in case of clip addition/removal
    /** todo only redraw track */
    updateVideoBitmap();
}

//////////////////////////////////////////////////////////////////////////
// MODEL EVENTS
//////////////////////////////////////////////////////////////////////////

/** @todo sequence::size event when adding clips, somewhere */

void VideoView::onVideoTracksAdded( model::EventAddVideoTracks& event )
{
    BOOST_FOREACH( model::TrackPtr track, event.getValue().addedTracks)
    {
        TrackView* t = new TrackView(track);
        t->initTimeline(&getTimeline());
        t->Bind(TRACK_UPDATE_EVENT, &VideoView::onVideoTrackUpdated, this);
    }
    updateVideoSize();
}

void VideoView::onVideoTracksRemoved( model::EventRemoveVideoTracks& event )
{
    updateVideoSize();
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

const wxBitmap& VideoView::getVideo() const
{
    return mVideo;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

int VideoView::requiredWidth()
{
    return
        std::max(std::max(
        getZoom().timeToPixels(5 * Constants::sMinute),            // Minimum width of 5 minutes
        getZoom().ptsToPixels(getSequence()->getNumberOfFrames())),    // At least enough to hold all clips
        getTimeline().GetClientSize().GetWidth());                           // At least the widget size
}

int VideoView::requiredVideoHeight()
{
    int requiredHeight = 0;
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getVideoTracks() )
    {
        requiredHeight += track->getHeight() + Constants::sTrackDividerHeight;
    }
    return requiredHeight;
}

void VideoView::getPositionInfo(wxPoint position, PointerPositionInfo& info )
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
}

void VideoView::updateVideoSize()
{
    mVideo.Create(requiredWidth(),requiredVideoHeight());
    updateVideoBitmap();
}

void VideoView::updateVideoBitmap()
{
    wxMemoryDC dc(mVideo);
    int y = 0;
    dc.SetBrush(Constants::sTrackDividerBrush);
    dc.SetPen(Constants::sTrackDividerPen);
    BOOST_REVERSE_FOREACH( model::TrackPtr track, getSequence()->getVideoTracks())
    {
        dc.DrawRectangle(0, y, dc.GetSize().GetWidth(), y + Constants::sTrackDividerHeight);
        y += Constants::sTrackDividerHeight;
        dc.DrawBitmap(getViewMap().getView(track)->getBitmap(), wxPoint(0,y));
        y += track->getHeight();
    }
    QueueEvent(new VideoUpdateEvent(this));
}

}} // namespace
