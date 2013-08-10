// Copyright 2013 Eric Raijmakers.
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

#include "Intervals.h"

#include "Config.h"
#include "Constants.h"
#include "Convert.h"
#include "Cursor.h"
#include "EmptyClip.h"
#include "IClip.h"
#include "IntervalChange.h"
#include "IntervalRemoveAll.h"
#include "IntervalsView.h"
#include "Layout.h"
#include "Menu.h"
#include "Scrolling.h"
#include "Sequence.h"
#include "Timeline.h"
#include "Track.h"
#include "TrimIntervals.h"
#include "UtilLog.h"
#include "UtilSerializeBoost.h"
#include "Zoom.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

PtsInterval makeInterval(pts a, pts b)
{
    VAR_INFO(a)(b);
    return PtsInterval(std::min(a,b),std::max(a,b));
}

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Intervals::Intervals(Timeline* timeline)
    :   Part(timeline)
    ,   mIntervals()
    ,   mNewIntervalActive(false)
    ,   mNewIntervalBegin(0)
    ,   mNewIntervalEnd(0)
    ,   mToggleActive(false)
    ,   mToggleBegin(0)
    ,   mToggleEnd(0)
{
    VAR_DEBUG(this);
}

Intervals::~Intervals()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

void Intervals::setView(IntervalsView* view)
{
    mView = view;
}

IntervalsView& Intervals::getView()
{
    ASSERT(mView);
    return *mView;
}

const IntervalsView& Intervals::getView() const
{
    ASSERT(mView);
    return *mView;
}

bool Intervals::isEmpty()
{
    return mIntervals.empty();
}

PtsIntervals Intervals::get()
{
    return mIntervals;
}

void Intervals::set(PtsIntervals intervals)
{
    PtsIntervals refresh = intervals + mIntervals; // +: Union
    mIntervals = intervals;
    VAR_INFO(mIntervals);
    getMenuHandler().updateItems();
    BOOST_FOREACH( PtsInterval i, refresh )
    {
        getView().refreshInterval( i );
    }
}

void Intervals::removeAll()
{
    ASSERT(wxThread::IsMain());
    LOG_INFO;
    BOOST_FOREACH( PtsInterval i, mIntervals )
    {
        getView().refreshInterval( i );
    }
    set(PtsIntervals());
}

void Intervals::addBeginMarker()
{
    pts cursor = determineSnap(getZoom().pixelsToPts(getCursor().getPosition()));
    mNewIntervalActive = true;
    mNewIntervalBegin = cursor + model::Convert::timeToPts(Config::ReadDouble(Config::sPathMarkerBeginAddition) * model::Constants::sSecond);
    mNewIntervalEnd = cursor + model::Convert::timeToPts(Config::ReadDouble(Config::sPathMarkerEndAddition)   * model::Constants::sSecond);
}

void Intervals::addEndMarker()
{
    if (mNewIntervalActive)
    {
        (new command::IntervalChange(getSequence(), makeInterval(mNewIntervalBegin, mNewIntervalEnd), true))->submit();
    }
    mNewIntervalActive = false;
}

void Intervals::startToggle()
{
    mToggleBegin = determineSnap(getZoom().pixelsToPts(getCursor().getPosition()));
    mToggleEnd = mToggleBegin;
    mToggleActive = true;
}

void Intervals::endToggle()
{
    if (mToggleActive)
    {
        mToggleActive = false;
        (new command::IntervalChange(getSequence(), makeInterval(mToggleBegin,mToggleEnd), toggleIsAddition()))->submit();
    }
}

bool Intervals::toggleIsAddition() const
{
    PtsIntervals overlap = mIntervals & makeInterval(mToggleBegin,mToggleBegin+1); // &: Intersection
    return overlap.empty();
}

void Intervals::update(pixel newCursorPosition)
{
    pts cursor = determineSnap(getZoom().pixelsToPts(newCursorPosition));
    if (mNewIntervalActive)
    {
        mNewIntervalEnd = cursor +  model::Convert::timeToPts(Config::ReadDouble(Config::sPathMarkerEndAddition) * model::Constants::sSecond);
        getView().refreshInterval(makeInterval(mNewIntervalBegin,mNewIntervalEnd));
    }
    if (mToggleActive)
    {
        mToggleEnd = cursor;
        getView().refreshInterval(makeInterval(mToggleBegin,mToggleEnd));
    }
}

void Intervals::change(PtsInterval interval, bool add)
{
    if (add)
    {
        mIntervals += interval;
    }
    else
    {
        mIntervals -= interval;
    }
    VAR_INFO(mIntervals);
    getMenuHandler().updateItems();
    getView().refreshInterval(interval);
    getTimeline().Update();
}

void Intervals::clear()
{
    (new command::IntervalRemoveAll(getSequence()))->submit();
}

PtsIntervals Intervals::getIntervalsForDrawing() const
{
    PtsIntervals intervals = mIntervals;

    if (mNewIntervalActive)
    {
        intervals += makeInterval(mNewIntervalBegin,mNewIntervalEnd);
    }
    if (mToggleActive)
    {
        if (toggleIsAddition())
        {
            intervals += makeInterval(mToggleBegin,mToggleEnd);
        }
        else
        {
            intervals -= makeInterval(mToggleBegin,mToggleEnd);
        }
    }
    return intervals;
}

//////////////////////////////////////////////////////////////////////////
// ACTIONS ON THE MARKED AREAS
//////////////////////////////////////////////////////////////////////////

void Intervals::deleteMarked()
{
     VAR_INFO(mIntervals);
    (new command::TrimIntervals(getSequence(), mIntervals,  _("Remove marked regions")))->submit();
}

void Intervals::deleteUnmarked()
{
    PtsIntervals unmarked;
    unmarked += PtsInterval(0,getSequence()->getLength());
    unmarked -= mIntervals;
    VAR_INFO(unmarked);
    (new command::TrimIntervals(getSequence(), unmarked, _("Remove unmarked regions")))->submit();
}

void Intervals::deleteEmpty()
{
    PtsIntervals empty;
    empty += PtsInterval(0,getSequence()->getLength());
    removeRegionUsedByClips(getSequence(),empty);
    VAR_INFO(empty);
    (new command::TrimIntervals(getSequence(), empty, _("Remove empty regions")))->submit();
}

void Intervals::deleteEmptyClip(model::IClipPtr clip)
{
    ASSERT(clip->isA<model::EmptyClip>());
    PtsIntervals empty;
    empty += PtsInterval(clip->getLeftPts(), clip->getRightPts());
    removeRegionUsedByClips(getSequence(),empty);
    VAR_INFO(empty);
    (new command::TrimIntervals(getSequence(), empty, _("Remove empty region")))->submit();
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

pts Intervals::determineSnap(pts position) const
{
    if (!Config::ReadBool(Config::sPathSnapClips))
    {
        return position;
    }

    pts snapAdjust = Layout::SnapDistance + 1;

    auto adjustSnap = [&snapAdjust,position](pts snappoint)
    {
        pts diff = position - snappoint;
        if ( (abs(diff)  <= Layout::SnapDistance) && (abs(diff) < abs(snapAdjust)))
        {
            snapAdjust = diff;
        }
    };

    BOOST_FOREACH( model::TrackPtr track, getSequence()->getTracks() )
    {
        model::IClipPtr clip = track->getClip(position);
        if (!clip)
        {
            // Must be beyond track length
            adjustSnap(track->getLength()); // right pts of rightmost clip
        }
        else
        {
            adjustSnap(clip->getLeftPts());
            adjustSnap(clip->getRightPts());
        }
    }
    if (snapAdjust != Layout::SnapDistance + 1)
    {
        return position - snapAdjust;
    }
    return position;
}

void Intervals::removeRegionUsedByClips(model::SequencePtr sequence, PtsIntervals& intervals)
{
    BOOST_FOREACH( model::TrackPtr track, sequence->getTracks() )
    {
        BOOST_FOREACH( model::IClipPtr clip, track->getClips() )
        {
            if (!clip->isA<model::EmptyClip>())
            {
                intervals -= PtsInterval(clip->getLeftPts(), clip->getRightPts());
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Intervals::serialize(Archive & ar, const unsigned int version)
{
    ar & mIntervals;
}

template void Intervals::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Intervals::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

}} // namespace