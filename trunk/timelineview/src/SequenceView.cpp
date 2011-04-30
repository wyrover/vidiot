#include "SequenceView.h"

#include <wx/dcclient.h>
#include <algorithm>
#include <wx/dcmemory.h>
#include "AudioView.h"
#include "Drag.h"
#include "Cursor.h"
#include "Intervals.h"
#include "Layout.h"
#include "VideoView.h"
#include "PositionInfo.h"
#include "Timeline.h"
#include "UtilLog.h"
#include "Zoom.h"
#include "Constants.h"
#include "Sequence.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

SequenceView::SequenceView(View* parent)
:   View(parent)
,   mVideoView(new VideoView(this))
,   mAudioView(new AudioView(this))
{
    VAR_DEBUG(this);

    // Ensure that for newly opened timelines the initial position is ok
    resetDividerPosition();

    getSequence()->Bind(model::EVENT_ADD_VIDEO_TRACK, &SequenceView::onVideoTracksAdded, this);
}

SequenceView::~SequenceView()
{
    VAR_DEBUG(this);

    getSequence()->Unbind(model::EVENT_ADD_VIDEO_TRACK, &SequenceView::onVideoTracksAdded, this);

    delete mAudioView;      mAudioView = 0;
    delete mVideoView;      mVideoView = 0;
}

//////////////////////////////////////////////////////////////////////////
// MODEL EVENTS
//////////////////////////////////////////////////////////////////////////

void SequenceView::onVideoTracksAdded( model::EventAddVideoTracks& event )
{
    resetDividerPosition();
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

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

pixel SequenceView::requiredWidth() const
{
    return
        std::max(
            std::max(
                getWindow().GetClientSize().GetWidth(),                         // At least the widget size
                getZoom().timeToPixels(5 * model::Constants::sMinute)),         // Minimum width of 5 minutes
                getZoom().ptsToPixels(getSequence()->getLength()));             // At least enough to hold all clips
}

pixel SequenceView::requiredHeight() const
{
    return
        std::max(
        getWindow().GetClientSize().GetHeight(),        // At least the widget size
        Layout::sTimeScaleHeight +
        Layout::sMinimalGreyAboveVideoTracksHeight +
        getVideo().getHeight() +
        Layout::sAudioVideoDividerHeight +
        getAudio().getHeight() +
        Layout::sMinimalGreyBelowAudioTracksHeight);    // Height of all combined components
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
    int minimum = Layout::sVideoPosition + getVideo().getHeight();
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
    return getSequence()->getDividerPosition() + Layout::sAudioVideoDividerHeight;
}

int SequenceView::getVideoPosition() const
{
    return getSequence()->getDividerPosition() - getVideo().getHeight();
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
    dc.SetPen(Layout::sBackgroundPen);
    dc.SetBrush(Layout::sBackgroundBrush);
    dc.DrawRectangle(0,0,w,h);

    // Draw timescale
    dc.SetBrush(wxNullBrush);
    dc.SetPen(Layout::sTimeScaleDividerPen);
    dc.DrawRectangle(0,0,w,Layout::sTimeScaleHeight);

    dc.SetFont(*Layout::sTimeScaleFont);

    // Draw seconds and minutes lines
    for (int ms = 0; getZoom().timeToPixels(ms) <= w; ms += model::Constants::sSecond)
    {
        int position = getZoom().timeToPixels(ms);
        bool isMinute = (ms % model::Constants::sMinute == 0);
        int height = Layout::sTimeScaleSecondHeight;

        if (isMinute)
        {
            height = Layout::sTimeScaleMinutesHeight;
        }

        dc.DrawLine(position,0,position,height);

        if (ms == 0)
        {
            dc.DrawText( "0", 5, Layout::sTimeScaleMinutesHeight );
        }
        else
        {
            if (isMinute)
            {
                wxDateTime t(ms / model::Constants::sHour, (ms % model::Constants::sHour) / model::Constants::sMinute, (ms % model::Constants::sMinute) / model::Constants::sSecond, ms % model::Constants::sSecond);
                wxString s = t.Format("%H:%M:%S.%l");
                wxSize ts = dc.GetTextExtent(s);
                dc.DrawText( s, position - ts.GetX() / 2, Layout::sTimeScaleMinutesHeight );
            }
        }
    }

    dc.DrawBitmap(getVideo().getBitmap(),   wxPoint(0,getVideoPosition()));

    dc.SetBrush(Layout::sAudioVideoDividerBrush);
    dc.SetPen(Layout::sAudioVideoDividerPen);
    dc.DrawRectangle(wxPoint(0,getSequence()->getDividerPosition()),wxSize(getSequenceView().getWidth(), Layout::sAudioVideoDividerHeight));

    dc.DrawBitmap(getAudio().getBitmap(),   wxPoint(0,getAudioPosition()));

    getIntervals().draw(dc);
}

}} // namespace
