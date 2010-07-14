#include "SelectIntervals.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "GuiTimeLine.h"
#include "GuiTimeLineZoom.h"
#include "UtilLog.h"
#include "Constants.h"
#include "GuiOptions.h"
#include "GuiMain.h"
#include "TimelineIntervalChange.h"
#include <math.h>
#include "UtilSerializeWxwidgets.h"

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

SelectIntervals::SelectIntervals()
:	mTimeline(0)
,   mMarkedIntervals()
,   mNewIntervalActive(false)
,   mNewIntervalBegin(0)
,   mNewIntervalEnd(0)
,   mToggleActive(false)
,   mToggleBegin(0)
,   mToggleEnd(0)
{
}

SelectIntervals::~SelectIntervals()
{
}

void SelectIntervals::init(GuiTimeLinePtr timeline)
{
    mTimeline = timeline;
    mTimeline->Bind(TIMELINE_CURSOR_MOVED, &SelectIntervals::onCursorMoved, this);
}

//////////////////////////////////////////////////////////////////////////
// MARKING / TOGGLING INTERFACE
//////////////////////////////////////////////////////////////////////////

void SelectIntervals::addBeginMarker()
{
    long c = mTimeline->mZoom->pixelsToPts(mTimeline->mCursorPosition);
    long b = c + mTimeline->mZoom->timeToPts(GuiOptions::getMarkerBeginAddition() * Constants::sSecond);
    long e = c + mTimeline->mZoom->timeToPts(GuiOptions::getMarkerEndAddition()   * Constants::sSecond);

    mNewIntervalActive = true;
    mNewIntervalBegin = b;
    mNewIntervalEnd = e;
}

void SelectIntervals::addEndMarker()
{
    if (mNewIntervalActive)
    {
        wxGetApp().getProject()->Submit(new command::TimelineIntervalChange(shared_from_this(), mNewIntervalBegin, mNewIntervalEnd, true));
    }
    mNewIntervalActive = false;
}

void SelectIntervals::startToggle()
{
    mToggleBegin = mTimeline->mZoom->pixelsToPts(mTimeline->mCursorPosition);
    mToggleEnd = mToggleBegin;
    mToggleActive = true;
}

void SelectIntervals::endToggle()
{
    if (mToggleActive)
    {
        wxRect r(makeRect(mToggleBegin,mToggleEnd));
        wxGetApp().getProject()->Submit(new command::TimelineIntervalChange(shared_from_this(), mToggleBegin, mToggleEnd, (mMarkedIntervals.Contains(r) == wxOutRegion)));
    }
    mToggleActive = false;
}

void SelectIntervals::change(long begin, long end, bool add)
{
    wxRect r(makeRect(begin,end));
    if (add)
    {
        mMarkedIntervals.Union(r);
    }
    else
    {
        mMarkedIntervals.Subtract(r);
    }
    refresh(begin,end);
}

//////////////////////////////////////////////////////////////////////////
// DRAWING
//////////////////////////////////////////////////////////////////////////

void SelectIntervals::draw(wxDC& dc)
{
    wxRegion drawRegion(mMarkedIntervals);

    VAR_DEBUG(mNewIntervalActive)(mNewIntervalBegin)(mNewIntervalEnd)(mToggleActive)(mToggleBegin)(mToggleEnd);

    if (mNewIntervalActive)
    {
        wxRect r(makeRect(mNewIntervalBegin,mNewIntervalEnd));
        drawRegion.Union(r);
    }
    if (mToggleActive)
    {
        wxRect r(makeRect(mToggleBegin,mToggleEnd));
        if (mMarkedIntervals.Contains(r) == wxOutRegion)
        {
            drawRegion.Union(r);

        }
        else
        {
            drawRegion.Subtract(r);
        }
    }

    wxRegionIterator it(drawRegion);
    while (it)
    {
        dc.DrawRectangle(ptsToPixels(it.GetRect()));
        it++;
    }
} 

//////////////////////////////////////////////////////////////////////////
// EVENTS
//////////////////////////////////////////////////////////////////////////

void SelectIntervals::onCursorMoved(EventTimelineCursorMoved& event)
{
    if (mNewIntervalActive)
    {
        mNewIntervalEnd = mTimeline->mZoom->pixelsToPts(event.getValue()) +  mTimeline->mZoom->timeToPts(GuiOptions::getMarkerEndAddition() * Constants::sSecond);
        refresh(mNewIntervalBegin,mNewIntervalEnd);
    }
    if (mToggleActive)
    {
        mToggleEnd = mTimeline->mZoom->pixelsToPts(event.getValue());
        refresh(mToggleBegin,mToggleEnd);
    }
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

wxRect SelectIntervals::makeRect(long x1, long x2)
{
    return wxRect(std::min(x1,x2),Constants::sTimeScaleHeight,std::abs(x2 - x1) + 1,Constants::sMinimalGreyAboveVideoTracksHeight);
}

wxRect SelectIntervals::ptsToPixels(wxRect rect)
{
    rect.x = mTimeline->mZoom->ptsToPixels(rect.x);
    rect.width = mTimeline->mZoom->ptsToPixels(rect.width);
    return rect;
}

void SelectIntervals::refresh(long begin, long end)
{
    wxRect r(ptsToPixels(makeRect(begin,end)));
    
    // Adjust for scrolling
    r.x -= mTimeline->getScrollOffset().x;
    r.y -= mTimeline->getScrollOffset().y;

    // enlargement to ensure that the vertical black end line of adjacent rects will be (re)drawn. Typical use: remove in the middle of an interval.
    r.x -= 1;
    r.width += 2;
    mTimeline->RefreshRect(r);
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void SelectIntervals::serialize(Archive & ar, const unsigned int version)
{
    ar & mMarkedIntervals;
}
template void SelectIntervals::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void SelectIntervals::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);


