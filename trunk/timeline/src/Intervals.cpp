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

#include "Intervals.h"

#include "Config.h"
#include "Constants.h"
#include "Convert.h"
#include "Cursor.h"
#include "EmptyClip.h"
#include "IClip.h"
#include "IntervalChange.h"
#include "IntervalRemoveAll.h"
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

void Intervals::set(const PtsIntervals& intervals)
{
    PtsIntervals refresh = intervals + mIntervals; // +: Union
    mIntervals = intervals;
    VAR_INFO(mIntervals);
    getMenuHandler().updateItems();
    for ( PtsInterval i : refresh )
    {
        refreshInterval( i );
    }
}

void Intervals::removeAll()
{
    ASSERT(wxThread::IsMain());
    LOG_INFO;
    for ( PtsInterval i : mIntervals )
    {
        refreshInterval( i );
    }
    set(PtsIntervals());
}

void Intervals::addBeginMarker()
{
    pts cursor = determineSnap(getCursor().getLogicalPosition());
    mNewIntervalActive = true;
    mNewIntervalBegin = cursor + model::Convert::timeToPts(Config::ReadDouble(Config::sPathTimelineMarkerBeginAddition) * model::Constants::sSecond);
    mNewIntervalEnd = cursor + model::Convert::timeToPts(Config::ReadDouble(Config::sPathTimelineMarkerEndAddition)   * model::Constants::sSecond);
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
    mToggleBegin = determineSnap(getCursor().getLogicalPosition());
    mToggleEnd = mToggleBegin;
    VAR_DEBUG(mToggleBegin)(mToggleEnd);
    mToggleActive = true;
}

void Intervals::endToggle()
{
    if (mToggleActive)
    {
        mToggleActive = false;
        VAR_DEBUG(mToggleBegin)(mToggleEnd);
        (new command::IntervalChange(getSequence(), makeInterval(mToggleBegin,mToggleEnd), toggleIsAddition()))->submit();
    }
}

bool Intervals::toggleIsAddition() const
{
    PtsIntervals overlap = mIntervals & makeInterval(mToggleBegin,mToggleBegin+1); // &: Intersection
    return overlap.empty();
}

void Intervals::update(pts newCursorPosition)
{
    pts cursor = determineSnap(newCursorPosition);
    VAR_DEBUG(cursor)(getCursor().getLogicalPosition());
    if (mNewIntervalActive)
    {
        mNewIntervalEnd = cursor +  model::Convert::timeToPts(Config::ReadDouble(Config::sPathTimelineMarkerEndAddition) * model::Constants::sSecond);
        refreshInterval(makeInterval(mNewIntervalBegin,mNewIntervalEnd));
    }
    if (mToggleActive)
    {
        mToggleEnd = cursor;
        refreshInterval(makeInterval(mToggleBegin,mToggleEnd));
    }
}

void Intervals::change(const PtsInterval& interval, bool add)
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
    refreshInterval(interval);
    getTimeline().Update();
}

void Intervals::clear()
{
    (new command::IntervalRemoveAll(getSequence()))->submit();
}

//////////////////////////////////////////////////////////////////////////
// DRAWING
//////////////////////////////////////////////////////////////////////////

void Intervals::draw(wxDC& dc, const wxRegion& region, const wxPoint& offset) const
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

    if (!intervals.empty())
    {
        dc.SetPen(wxPen{ wxColour{ 128, 128, 128 } }); // 128,128,128 ==> wxGREY
        dc.SetBrush(wxBrush{ wxColour{ 211, 211, 211 }, wxBRUSHSTYLE_CROSSDIAG_HATCH}); // 211,211,211 ==> wxLIGHT_GREY
    }
    for ( PtsInterval i : intervals )
    {
        wxRect r(makeRect(i));
        r.Offset(-offset);
        dc.DrawRectangle(r);
    }
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

void Intervals::deleteEmptyClip(const model::IClipPtr& clip)
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
    if (!Config::ReadBool(Config::sPathTimelineSnapClips))
    {
        return position;
    }

    pts snapAdjust = Timeline::SnapDistance + 1;

    auto adjustSnap = [&snapAdjust,position](pts snappoint)
    {
        pts diff = position - snappoint;
        if ( (abs(diff)  <= Timeline::SnapDistance) && (abs(diff) < abs(snapAdjust)))
        {
            snapAdjust = diff;
        }
    };

    for ( model::TrackPtr track : getSequence()->getTracks() )
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
    if (snapAdjust != Timeline::SnapDistance + 1)
    {
        return position - snapAdjust;
    }
    return position;
}

void Intervals::removeRegionUsedByClips(const model::SequencePtr& sequence, PtsIntervals& intervals)
{
    for ( model::TrackPtr track : sequence->getTracks() )
    {
        for ( model::IClipPtr clip : track->getClips() )
        {
            if (!clip->isA<model::EmptyClip>())
            {
                intervals -= PtsInterval(clip->getLeftPts(), clip->getRightPts());
            }
        }
    }
}

void Intervals::refreshInterval(const PtsInterval& interval)
{
    wxRect r(makeRect(interval));
    r.x -= 1;
    r.width += 2; // enlargement to ensure that the vertical black end line of adjacent rects will be (re)drawn. Typical use: remove in the middle of an interval.
    getTimeline().repaint(r);
}

wxRect Intervals::makeRect(const PtsInterval& interval) const
{
    PixelInterval pixels( getZoom().ptsToPixels(interval.lower()), getZoom().ptsToPixels(interval.upper()) );
    return wxRect(pixels.lower(),0,pixels.upper() - pixels.lower() + 1,getSequenceView().getSize().GetHeight());
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Intervals::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_NVP(mIntervals);
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}

template void Intervals::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void Intervals::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

}} // namespace
