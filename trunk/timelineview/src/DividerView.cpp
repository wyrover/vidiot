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

#include "DividerView.h"

#include "Layout.h"
#include "Track.h"
#include "TrackView.h"
#include "Sequence.h"
#include "PositionInfo.h"
#include "ViewMap.h"
#include "SequenceView.h"
#include "SequenceEvent.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

DividerView::DividerView(View* parent, pixel height, model::TrackPtr track)
:   View(parent)
,   mHeight(height)
,   mTrack(track)
,   mY(boost::none)
,   mBitmap(boost::none)
{
    VAR_DEBUG(this)(height);
    getViewMap().registerDivider(mTrack,this);
}

DividerView::~DividerView()
{
    VAR_DEBUG(this);
    getViewMap().unregisterDivider(mTrack);
}

//////////////////////////////////////////////////////////////////////////
// VIEW
//////////////////////////////////////////////////////////////////////////

pixel DividerView::getX() const
{
    return getParent().getX();
}

pixel DividerView::getY() const
{
    if (!mY)
    {
        if (!mTrack)
        {
            mY.reset(getSequence()->getDividerPosition());
        }
        else if (mTrack->isA<model::VideoTrack>())
        {
            mY.reset(getViewMap().getView(mTrack)->getY() - Layout::TrackDividerHeight);
        }
        else
        {
            mY.reset(getViewMap().getView(mTrack)->getY() + getViewMap().getView(mTrack)->getH());
        }
    }
    return *mY;
}

pixel DividerView::getW() const
{
    return getParent().getW();
}

pixel DividerView::getH() const
{
    return mHeight;
}

void DividerView::invalidateRect()
{
    mY.reset();
    mBitmap.reset();

}

void DividerView::draw(wxDC& dc, const wxRegion& region, const wxPoint& offset) const
{
    if (!mBitmap)
    {
        mBitmap.reset(wxBitmap(getSize()));
        wxMemoryDC dc(*mBitmap);
        getTimeline().drawDivider(dc, 0, mHeight);
    }
    getTimeline().copyRect(dc,region,offset, *mBitmap, getRect());
}

}} // namespace