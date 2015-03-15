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
#include "Constants.h"
#include "Convert.h"
#include "Cursor.h"
#include "Drag.h"
#include "Intervals.h"
#include "Layout.h"
#include "PositionInfo.h"
#include "Sequence.h"
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

SequenceView::SequenceView(Timeline* timeline)
: View(timeline)
, mTimescaleView(new TimescaleView(this))
, mVideoView(new VideoView(this))
, mDividerView(new DividerView(this, Layout::AudioVideoDividerHeight))
, mAudioView(new AudioView(this))
, mWidth(boost::none)
, mHeight(boost::none)
, mMinimumLength(0)
{
    VAR_DEBUG(this);
}

SequenceView::~SequenceView()
{
    VAR_DEBUG(this);

    delete mAudioView;      mAudioView = 0;
    delete mDividerView;    mDividerView = 0;
    delete mVideoView;      mVideoView = 0;
    delete mTimescaleView;  mTimescaleView = 0;
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
    if (!mWidth)
    {
        pts length = std::max(getDefaultLength(), mMinimumLength); // At least the fixed minimum length
        mWidth.reset(std::max(
            getTimeline().GetClientSize().GetWidth(),   // At least the widget size
            getZoom().ptsToPixels(length)               // At least enough to hold all clips
            ));                           
    }
    return *mWidth;
}

pixel SequenceView::getH() const
{
    if (!mHeight)
    {
        int height =
            std::max(
            getTimeline().GetClientSize().GetHeight(),      // At least the widget size
            Layout::TimeScaleHeight +
            Layout::MinimalGreyAboveVideoTracksHeight +
            mVideoView->getH() +
            Layout::AudioVideoDividerHeight +
            mAudioView->getH() +
            Layout::MinimalGreyBelowAudioTracksHeight);     // Height of all combined components
        mHeight.reset(height);
    }
    return *mHeight;
}

void SequenceView::invalidateRect()
{
    mWidth.reset();
    mHeight.reset();
    mTimescaleView->invalidateRect();
    mDividerView->invalidateRect();
    mAudioView->invalidateRect();
    mVideoView->invalidateRect();
}

void SequenceView::draw(wxDC& dc, const wxRegion& region, const wxPoint& offset) const
{
    if (getX() > 0)
    {
        dc.SetPen(Layout::get().BackgroundPen);
        dc.SetBrush(Layout::get().BackgroundBrush);
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
    int minimum = Layout::VideoPosition + getVideo().getH();
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
    return getSequence()->getLength() + model::Convert::timeToPts(10 * model::Constants::sSecond); // Add 10 extra seconds
}

void SequenceView::setMinimumLength(pts length)
{
    mMinimumLength = length;
    getTimeline().resize();
}

}} // namespace
