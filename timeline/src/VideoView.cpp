#include "VideoView.h"

#include <wx/dc.h>
#include <wx/dcmemory.h>
#include <boost/foreach.hpp>
#include "Constants.h"
#include "Track.h"
#include "TrackView.h"
#include "Sequence.h"
#include "ViewMap.h"
#include "PositionInfo.h"
#include "Timeline.h"
#include "Divider.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

VideoView::VideoView(View* parent)
:   View(parent)
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
// MODEL EVENTS
//////////////////////////////////////////////////////////////////////////

void VideoView::onVideoTracksAdded( model::EventAddVideoTracks& event )
{
    BOOST_FOREACH( model::TrackPtr track, event.getValue().addedTracks)
    {
        TrackView* t = new TrackView(track,this);
    }
    invalidateBitmap();
}

void VideoView::onVideoTracksRemoved( model::EventRemoveVideoTracks& event )
{
    invalidateBitmap();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

pixel VideoView::requiredWidth() const
{
    return getParent().requiredWidth();
}

pixel VideoView::requiredHeight() const
{
    pixel requiredHeight = 0;
    BOOST_FOREACH( model::TrackPtr track, getSequence()->getVideoTracks() )
    {
        requiredHeight += track->getHeight() + Constants::sTrackDividerHeight;
    }
    return requiredHeight;
}

void VideoView::getPositionInfo(wxPoint position, PointerPositionInfo& info ) const
{
    int top = getDivider().getVideoPosition();
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

void VideoView::draw(wxBitmap& bitmap) const
{
    wxMemoryDC dc(bitmap);
    int y = 0;
    BOOST_REVERSE_FOREACH( model::TrackPtr track, getSequence()->getVideoTracks())
    {
        dc.SetBrush(Constants::sTrackDividerBrush);
        dc.SetPen(Constants::sTrackDividerPen);
        dc.DrawRectangle(0, y, dc.GetSize().GetWidth(), Constants::sTrackDividerHeight);
        y += Constants::sTrackDividerHeight;
        dc.DrawBitmap(getViewMap().getView(track)->getBitmap(), wxPoint(0,y));
        y += track->getHeight();
    }
}

}} // namespace
