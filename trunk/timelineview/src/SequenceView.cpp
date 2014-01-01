// Copyright 2013,2014 Eric Raijmakers.
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

#include "SequenceView.h"

#include "AudioTrack.h"
#include "AudioView.h"
#include "Constants.h"
#include "Convert.h"
#include "Cursor.h"
#include "Drag.h"
#include "Intervals.h"
#include "IntervalsView.h"
#include "Layout.h"
#include "ModelEvent.h"
#include "PositionInfo.h"
#include "Sequence.h"
#include "SequenceEvent.h"
#include "Timeline.h"
#include "TimescaleView.h"
#include "UtilLog.h"
#include "VideoTrack.h"
#include "VideoView.h"
#include "Zoom.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

SequenceView::SequenceView(View* parent)
:   View(parent)
,   mTimescaleView(new TimescaleView(this))
,   mVideoView(new VideoView(this))
,   mAudioView(new AudioView(this))
{
    VAR_DEBUG(this);

    getSequence()->Bind(model::EVENT_LENGTH_CHANGED, &SequenceView::onSequenceLengthChanged, this);
}

SequenceView::~SequenceView()
{
    VAR_DEBUG(this);

    getSequence()->Unbind(model::EVENT_LENGTH_CHANGED, &SequenceView::onSequenceLengthChanged, this);

    delete mAudioView;      mAudioView = 0;
    delete mVideoView;      mVideoView = 0;
    delete mTimescaleView;  mTimescaleView = 0;
}

//////////////////////////////////////////////////////////////////////////
// MODEL EVENTS
//////////////////////////////////////////////////////////////////////////

void SequenceView::onSequenceLengthChanged(model::EventLengthChanged& event)
{
    canvasResized();
    event.Skip();
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

void SequenceView::canvasResized()
{
    getTimeline().resize(); // Note that this also triggers enabling/disabling of scrollbars if required
    invalidateBitmap();
    mTimescaleView->canvasResized();
    mAudioView->canvasResized();
    mVideoView->canvasResized();
}

pixel SequenceView::minimumWidth() const
{
    pts length =
        getSequence()->getLength() +
        model::Convert::timeToPts(10 * model::Constants::sSecond); // Add 10 extra seconds
    return
        std::max(
        getTimeline().GetClientSize().GetWidth(),   // At least the widget size
        getZoom().ptsToPixels(length));             // At least enough to hold all clips
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

    drawDivider(dc, getSequence()->getDividerPosition(), Layout::AudioVideoDividerHeight);

    dc.DrawBitmap(getAudio().getBitmap(),   wxPoint(0,getAudioPosition()));
}

}} // namespace