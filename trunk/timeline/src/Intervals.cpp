#include "Intervals.h"

#include <math.h>
#include <wx/confbase.h>
#include <wx/dcmemory.h>
#include <boost/foreach.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "Config.h"
#include "Constants.h"
#include "Convert.h"
#include "Cursor.h"
#include "IntervalChange.h"
#include "IntervalRemoveAll.h"
#include "Menu.h"
#include "Project.h"
#include "Scrolling.h"
#include "SequenceView.h"
#include "Timeline.h"
#include "TrimIntervals.h"
#include "UtilLog.h"
#include "UtilSerializeBoost.h"
#include "Zoom.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Intervals::Intervals(Timeline* timeline)
:   Part(timeline)
,   mMarkedIntervals()
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
// MARKING / TOGGLING INTERFACE
//////////////////////////////////////////////////////////////////////////

bool Intervals::isEmpty()
{
    return mMarkedIntervals.empty();
}

PtsIntervals Intervals::get()
{
    return mMarkedIntervals;
}

void Intervals::set(PtsIntervals intervals)
{
    PtsIntervals refresh = intervals + mMarkedIntervals; // +: Union
    mMarkedIntervals = intervals;
    VAR_INFO(mMarkedIntervals);
    getMenuHandler().updateItems();
    BOOST_FOREACH( PtsInterval i, refresh )
    {
        refreshInterval( i );
    }
}

void Intervals::removeAll()
{
    LOG_INFO;
    set(PtsIntervals());
    getTimeline().getSequenceView().resetDividerPosition(); // trigger redraw of the sequence view
}

void Intervals::addBeginMarker()
{
    pts cursor = getZoom().pixelsToPts(getCursor().getPosition());
    mNewIntervalActive = true;
    mNewIntervalBegin = cursor + model::Convert::timeToPts(Config::ReadDouble(Config::sPathMarkerBeginAddition) * model::Constants::sSecond);
    mNewIntervalEnd = cursor + model::Convert::timeToPts(Config::ReadDouble(Config::sPathMarkerEndAddition)   * model::Constants::sSecond);
}

void Intervals::addEndMarker()
{
    if (mNewIntervalActive)
    {
        model::Project::get().Submit(new command::IntervalChange(getSequence(), makeInterval(mNewIntervalBegin, mNewIntervalEnd), true));
    }
    mNewIntervalActive = false;
}

void Intervals::startToggle()
{
    mToggleBegin = getZoom().pixelsToPts(getCursor().getPosition());
    mToggleEnd = mToggleBegin;
    mToggleActive = true;
}

void Intervals::endToggle()
{
    if (mToggleActive)
    {
        model::Project::get().Submit(new command::IntervalChange(getSequence(), makeInterval(mToggleBegin,mToggleEnd), toggleIsAddition()));
    }
    mToggleActive = false;
}

bool Intervals::toggleIsAddition() const
{
    PtsIntervals overlap = mMarkedIntervals & makeInterval(mToggleBegin,mToggleBegin+1); // &: Intersection
    return overlap.empty();
}

void Intervals::update(pixel newCursorPosition)
{
    pts cursor = getZoom().pixelsToPts(newCursorPosition);
    if (mNewIntervalActive)
    {
        mNewIntervalEnd = cursor +  model::Convert::timeToPts(wxConfigBase::Get()->ReadDouble(Config::sPathMarkerEndAddition, 0) * model::Constants::sSecond);
        refreshInterval(makeInterval(mNewIntervalBegin,mNewIntervalEnd));
    }
    if (mToggleActive)
    {
        mToggleEnd = cursor;
        refreshInterval(makeInterval(mToggleBegin,mToggleEnd));
    }
}

void Intervals::change(PtsInterval interval, bool add)
{
    if (add)
    {
        mMarkedIntervals += interval;
    }
    else
    {
        mMarkedIntervals -= interval;
    }
    VAR_INFO(mMarkedIntervals);
    getMenuHandler().updateItems();
    refreshInterval(interval);
     getTimeline().Update();
}

void Intervals::clear()
{
    model::Project::get().Submit(new command::IntervalRemoveAll(getSequence(), mMarkedIntervals));
}

void Intervals::refresh()
{
}

//////////////////////////////////////////////////////////////////////////
// ACTIONS ON THE MARKED AREAS
//////////////////////////////////////////////////////////////////////////

void Intervals::deleteMarked()
{
    model::Project::get().Submit(new command::TrimIntervals(getSequence(), mMarkedIntervals, true));
}

void Intervals::deleteUnmarked()
{
    model::Project::get().Submit(new command::TrimIntervals(getSequence(), mMarkedIntervals, false));
}

//////////////////////////////////////////////////////////////////////////
// DRAWING
//////////////////////////////////////////////////////////////////////////

void Intervals::draw(wxDC& dc) const
{
    VAR_DEBUG(mNewIntervalActive)(mNewIntervalBegin)(mNewIntervalEnd)(mToggleActive)(mToggleBegin)(mToggleEnd);

    PtsIntervals intervals = mMarkedIntervals;

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

    wxBitmap bmp(2,2);
    wxMemoryDC dcM(bmp);
    dcM.SelectObject(wxNullBitmap);

    dc.SetPen(*wxGREY_PEN);
    wxBrush b(*wxLIGHT_GREY,wxBRUSHSTYLE_CROSSDIAG_HATCH);
    dc.SetBrush(b);

    BOOST_FOREACH( PtsInterval i, intervals )
    {
        dc.DrawRectangle(makeRect(i));
    }
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

PtsInterval Intervals::makeInterval(pts a, pts b) const
{
    return PtsInterval(std::min(a,b),std::max(a,b));
}

wxRect Intervals::makeRect(PtsInterval interval) const
{
    PixelInterval pixels(ptsToPixels(interval));
    return wxRect(pixels.lower(),0,pixels.upper() - pixels.lower() + 1,getSequenceView().getSize().GetHeight());
}

PixelInterval Intervals::ptsToPixels(PtsInterval interval) const
{
    return PixelInterval( getZoom().ptsToPixels(interval.lower()), getZoom().ptsToPixels(interval.upper()) );
}

void Intervals::refreshInterval(PtsInterval interval)
{
    wxRect r(makeRect(interval));

    // Adjust for scrolling
    r.x -= getScrolling().getOffset().x;
    r.y -= getScrolling().getOffset().y;

    // enlargement to ensure that the vertical black end line of adjacent rects will be (re)drawn. Typical use: remove in the middle of an interval.
    r.x -= 1;
    r.width += 2;
    getTimeline().RefreshRect(r);
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Intervals::serialize(Archive & ar, const unsigned int version)
{
    ar & mMarkedIntervals;
}
template void Intervals::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Intervals::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);
}} // namespace