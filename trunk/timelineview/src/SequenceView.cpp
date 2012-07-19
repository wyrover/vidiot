#include "SequenceView.h"

#include "AudioView.h"
#include "Constants.h"
#include "Cursor.h"
#include "Drag.h"
#include "Intervals.h"
#include "IntervalsView.h"
#include "Layout.h"
#include "PositionInfo.h"
#include "Sequence.h"
#include "Timeline.h"
#include "TimescaleView.h"
#include "UtilLog.h"
#include "VideoTrack.h"
#include "AudioTrack.h"
#include "VideoView.h"
#include "Zoom.h"
#include "SequenceEvent.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

SequenceView::SequenceView(View* parent)
:   View(parent)
,   mTimescaleView(new TimescaleView(this))
,   mVideoView(new VideoView(this))
,   mAudioView(new AudioView(this))
,   mIntervalsView(new IntervalsView(this))
{
    VAR_DEBUG(this);
}

SequenceView::~SequenceView()
{
    VAR_DEBUG(this);

    delete mIntervalsView;  mIntervalsView = 0;
    delete mAudioView;      mAudioView = 0;
    delete mVideoView;      mVideoView = 0;
    delete mTimescaleView;  mTimescaleView = 0;
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

TimescaleView& SequenceView::getTimescale()
{
    return *mTimescaleView;
}

const TimescaleView& SequenceView::getTimescale() const
{
    return *mTimescaleView;
}

VideoView& SequenceView::getVideo()
{
    return *mVideoView;
}

const VideoView& SequenceView::getVideo() const
{
    return *mVideoView;
}

AudioView& SequenceView::getAudio()
{
    return *mAudioView;
}

const AudioView& SequenceView::getAudio() const
{
    return *mAudioView;
}

pixel SequenceView::minimumWidth() const
{
    return
        std::max(
        std::max(
        getTimeline().GetClientSize().GetWidth(),                       // At least the widget size
        getZoom().timeToPixels(5 * model::Constants::sMinute)),         // Minimum width of 5 minutes
        getZoom().ptsToPixels(getSequence()->getLength()));             // At least enough to hold all clips
}

wxSize SequenceView::requiredSize() const
{
    int height =
        std::max(
        getTimeline().GetClientSize().GetHeight(),        // At least the widget size
        Layout::TimeScaleHeight +
        Layout::MinimalGreyAboveVideoTracksHeight +
        getVideo().getSize().GetHeight() +
        Layout::AudioVideoDividerHeight +
        getAudio().getSize().GetHeight() +
        Layout::MinimalGreyBelowAudioTracksHeight);    // Height of all combined components
    return wxSize(minimumWidth(),height);
}

void SequenceView::getPositionInfo(wxPoint position, PointerPositionInfo& info ) const
{
    info.onAudioVideoDivider =
        position.y >= getSequence()->getDividerPosition() &&
        position.y <= getAudioPosition();

    if (!info.onAudioVideoDivider)
    {
        getVideo().getPositionInfo(position, info);
        if (!info.track)
        {
            getAudio().getPositionInfo(position, info);
        }
    }
}

void SequenceView::setDividerPosition(int position)
{
    int minimum = Layout::VideoPosition + getVideo().getSize().GetHeight();
    if (position < minimum)
    {
        position = minimum;
    }
    getSequence()->setDividerPosition(position);
    invalidateBitmap();
    getTimeline().Update();
}

void SequenceView::resetDividerPosition()
{
    setDividerPosition(getSequence()->getDividerPosition());
}

int SequenceView::getAudioPosition() const
{
    return getSequence()->getDividerPosition() + Layout::AudioVideoDividerHeight;
}

int SequenceView::getVideoPosition() const
{
    return getSequence()->getDividerPosition() - getVideo().getSize().GetHeight();
}

pixel SequenceView::getPosition(model::TrackPtr track) const
{
    if (track->isA<model::VideoTrack>())
    {
        return getVideoPosition() + getVideo().getPosition(track);
    }
    if (track->isA<model::AudioTrack>())
    {
        return getAudioPosition() + getAudio().getPosition(track);
    }
    return -1;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void SequenceView::draw(wxBitmap& bitmap) const
{
    wxMemoryDC dc(bitmap);

    // Get size of canvas
    int w = bitmap.GetWidth();
    int h = bitmap.GetHeight();

    // Set BG
    dc.SetPen(Layout::get().BackgroundPen);
    dc.SetBrush(Layout::get().BackgroundBrush);
    dc.DrawRectangle(0,0,w,h);

    dc.DrawBitmap(getTimescale().getBitmap(), wxPoint(0,0));

    dc.DrawBitmap(getVideo().getBitmap(),   wxPoint(0,getVideoPosition()));

    dc.SetBrush(Layout::get().AudioVideoDividerBrush);
    dc.SetPen(Layout::get().AudioVideoDividerPen);
    dc.DrawRectangle(wxPoint(0,getSequence()->getDividerPosition()),wxSize(getSequenceView().getSize().GetWidth(), Layout::get().AudioVideoDividerHeight));

    dc.DrawBitmap(getAudio().getBitmap(),   wxPoint(0,getAudioPosition()));

    getIntervals().getView().draw(dc);
}

}} // namespace