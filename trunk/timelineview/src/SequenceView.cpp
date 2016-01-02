// Copyright 2013-2015 Eric Raijmakers.
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
#include "Convert.h"
#include "Cursor.h"
#include "DividerView.h"
#include "Drag.h"
#include "Intervals.h"
#include "ModelEvent.h"
#include "PositionInfo.h"
#include "Sequence.h"
#include "Timeline.h"
#include "TimescaleView.h"
#include "VideoTrack.h"
#include "VideoView.h"
#include "Zoom.h"

namespace gui { namespace timeline {

const pixel SequenceView::MinimalGreyAboveVideoTracksHeight{ 10 };
const pixel SequenceView::MinimalGreyBelowAudioTracksHeight{ 10 };

// todo bug: edit volume of clip (default key frame). Undo. Redo. The clip's previews (in timeline) are NOT updated!
//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

SequenceView::SequenceView(Timeline* timeline)
: View(timeline)
, mTimescaleView(new TimescaleView(this))
, mVideoView(new VideoView(this))
, mDividerView(new DividerView(this, DividerView::AudioVideoDividerHeight))
, mAudioView(new AudioView(this))
, mMinimumLength(0)
{
    VAR_DEBUG(this);
    getSequence()->Bind(model::EVENT_LENGTH_CHANGED, &SequenceView::onSequenceLengthChanged, this);
    getSequence()->Bind(model::EVENT_HEIGHT_CHANGED, &SequenceView::onSequenceHeightChanged, this);

}

SequenceView::~SequenceView()
{
    VAR_DEBUG(this);

    delete mAudioView;      mAudioView = 0;
    delete mDividerView;    mDividerView = 0;
    delete mVideoView;      mVideoView = 0;
    delete mTimescaleView;  mTimescaleView = 0;
    getSequence()->Unbind(model::EVENT_LENGTH_CHANGED, &SequenceView::onSequenceLengthChanged, this);
    getSequence()->Unbind(model::EVENT_HEIGHT_CHANGED, &SequenceView::onSequenceHeightChanged, this);
}

//////////////////////////////////////////////////////////////////////////
// VIEW
//////////////////////////////////////////////////////////////////////////

pixel SequenceView::getX() const
{
    return getTimeline().getShift();
}

pixel SequenceView::getY() const
{
    return 0;
}

pixel SequenceView::getW() const
{
    return getTimeline().GetVirtualSize().GetWidth();
}

pixel SequenceView::getH() const
{
    return getTimeline().GetVirtualSize().GetHeight();
}

void SequenceView::invalidateRect()
{
    mTimescaleView->invalidateRect();
    mDividerView->invalidateRect();
    mAudioView->invalidateRect();
    mVideoView->invalidateRect();
}

void SequenceView::draw(wxDC& dc, const wxRegion& region, const wxPoint& offset) const
{
    if (getX() > 0)
    {
        dc.SetPen(wxPen{ wxColour{ 212, 208, 200 } }); // Background colour
        dc.SetBrush(wxBrush{ wxColour{ 212, 208, 200 } }); // Background colour
        dc.DrawRectangle(0,0,getX(),getH());
    }

    mTimescaleView->draw(dc, region, offset);

    getTimeline().clearRect(dc, region, offset, wxRect(0, mTimescaleView->getH(), getW(), mVideoView->getY() - mTimescaleView->getH()));

    mVideoView->draw(dc, region, offset);
    mDividerView->draw(dc,region,offset);
    mAudioView->draw(dc, region, offset);

    pixel bottom = mAudioView->getY() + mAudioView->getH();
    getTimeline().clearRect(dc, region, offset, wxRect(0, bottom, getW(), getH() - bottom));
};

//////////////////////////////////////////////////////////////////////////
// MODEL EVENTS
//////////////////////////////////////////////////////////////////////////

void SequenceView::onSequenceLengthChanged(model::EventLengthChanged& event)
{
    getTimeline().resize();
    event.Skip();
}

void SequenceView::onSequenceHeightChanged(model::EventHeightChanged& event)
{

    getTimeline().resize();
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

void SequenceView::getPositionInfo(const wxPoint& position, PointerPositionInfo& info ) const
{
    if (position.y < getSequence()->getDividerPosition())
    {
        getVideo().getPositionInfo(position, info);
    }
    else if (position.y <= mAudioView->getY())
    {
        info.onAudioVideoDivider = true;
    }
    else
    {
        getAudio().getPositionInfo(position, info);
    }
}

void SequenceView::setDividerPosition(int position)
{
    invalidateRect();
    int minimum = TimescaleView::TimeScaleHeight + MinimalGreyAboveVideoTracksHeight + getVideo().getH();
    if (position < minimum)
    {
        position = minimum;
    }
    getSequence()->setDividerPosition(position);
    getTimeline().Refresh(false);
    getTimeline().Update();
}

void SequenceView::resetDividerPosition()
{
    setDividerPosition(getSequence()->getDividerPosition());
}

pts SequenceView::getDefaultLength() const
{
    return getSequence()->getLength() + model::Convert::timeToPts(10 * sSecond); // Add 10 extra seconds
}

void SequenceView::setMinimumLength(pts length)
{
    mMinimumLength = length;
    getTimeline().resize();
}

wxSize SequenceView::getDefaultSize() const
{
    pts length = std::max(getDefaultLength(), mMinimumLength); // At least the fixed minimum length
    int w{ std::max(
        getTimeline().GetClientSize().GetWidth(),   // At least the widget size
        getZoom().ptsToPixels(length)               // At least enough to hold all clips
        ) };
 
    int h{ std::max(
        getTimeline().GetClientSize().GetHeight(),      // At least the widget size
        getSequence()->getDividerPosition() +
        DividerView::AudioVideoDividerHeight +
        mAudioView->getH() +
        SequenceView::MinimalGreyBelowAudioTracksHeight) };     // Height of all combined components
    return wxSize{ w, h };
}

}} // namespace
