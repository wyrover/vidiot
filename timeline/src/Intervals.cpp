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
#include "IntervalsView.h"
#include "Menu.h"
#include "Project.h"
#include "Scrolling.h"
#include "Timeline.h"
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
    LOG_INFO;
    BOOST_FOREACH( PtsInterval i, mIntervals )
    {
        getView().refreshInterval( i );
    }
    set(PtsIntervals());
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
        mToggleActive = false;
        model::Project::get().Submit(new command::IntervalChange(getSequence(), makeInterval(mToggleBegin,mToggleEnd), toggleIsAddition()));
    }
}

bool Intervals::toggleIsAddition() const
{
    PtsIntervals overlap = mIntervals & makeInterval(mToggleBegin,mToggleBegin+1); // &: Intersection
    return overlap.empty();
}

void Intervals::update(pixel newCursorPosition)
{
    pts cursor = getZoom().pixelsToPts(newCursorPosition);
    if (mNewIntervalActive)
    {
        mNewIntervalEnd = cursor +  model::Convert::timeToPts(wxConfigBase::Get()->ReadDouble(Config::sPathMarkerEndAddition, 0) * model::Constants::sSecond);
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
    model::Project::get().Submit(new command::IntervalRemoveAll(getSequence()));
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
    model::Project::get().Submit(new command::TrimIntervals(getSequence(), mIntervals, true));
}

void Intervals::deleteUnmarked()
{
    model::Project::get().Submit(new command::TrimIntervals(getSequence(), mIntervals, false));
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