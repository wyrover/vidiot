#include "Intervals.h"

#include <math.h>
#include <boost/foreach.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "Timeline.h"
#include "Zoom.h"
#include "UtilLog.h"
#include "Constants.h"
#include "GuiOptions.h"
#include "GuiMain.h"
#include "Clip.h"
#include "TrackView.h"
#include "ClipView.h"
#include "Track.h"
#include "Project.h"
#include "TimelineIntervalChange.h"
#include "TimelineIntervalRemoveAll.h"
#include "UtilSerializeWxwidgets.h"
#include "ids.h"
#include "Menu.h"
#include "Cursor.h"
#include "ViewMap.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Intervals::Intervals(Timeline* timeline)
:   wxEvtHandler()
,   Part(timeline)
,   mMarkedIntervals()
,   mNewIntervalActive(false)
,   mNewIntervalBegin(0)
,   mNewIntervalEnd(0)
,   mToggleActive(false)
,   mToggleBegin(0)
,   mToggleEnd(0)
{
}

Intervals::~Intervals()
{
}

//////////////////////////////////////////////////////////////////////////
// MARKING / TOGGLING INTERFACE
//////////////////////////////////////////////////////////////////////////

bool Intervals::isEmpty()
{
    return mMarkedIntervals.IsEmpty();
}

wxRegion Intervals::get()
{
    return mMarkedIntervals;
}

void Intervals::set(wxRegion region)
{
    wxRect r = mMarkedIntervals.GetBox().Union(region.GetBox());
    mMarkedIntervals = region;
    getMenuHandler().update();
    refresh(r.x,r.x + r.width);
}

void Intervals::addBeginMarker()
{
    long c = getZoom().pixelsToPts(getCursor().getPosition());
    long b = c + getZoom().timeToPts(GuiOptions::getMarkerBeginAddition() * Constants::sSecond);
    long e = c + getZoom().timeToPts(GuiOptions::getMarkerEndAddition()   * Constants::sSecond);

    mNewIntervalActive = true;
    mNewIntervalBegin = b;
    mNewIntervalEnd = e;
}

void Intervals::addEndMarker()
{
    if (mNewIntervalActive)
    {
         wxGetApp().getProject()->Submit(new command::TimelineIntervalChange(getTimeline(), mNewIntervalBegin, mNewIntervalEnd, true));
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
        wxRect r(makeRect(mToggleBegin,mToggleEnd));
        wxGetApp().getProject()->Submit(new command::TimelineIntervalChange(getTimeline(), mToggleBegin, mToggleEnd, (mMarkedIntervals.Contains(r) == wxOutRegion)));
    }
    mToggleActive = false;
}

void Intervals::update(long newCursorPosition)
{
    if (mNewIntervalActive)
    {
        mNewIntervalEnd = getZoom().pixelsToPts(newCursorPosition) +  getZoom().timeToPts(GuiOptions::getMarkerEndAddition() * Constants::sSecond);
        refresh(mNewIntervalBegin,mNewIntervalEnd);
    }
    if (mToggleActive)
    {
        mToggleEnd = getZoom().pixelsToPts(newCursorPosition);
        refresh(mToggleBegin,mToggleEnd);
    }
}

void Intervals::change(long begin, long end, bool add)
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
    getMenuHandler().update();
    refresh(begin,end);
}

void Intervals::clear()
{
    wxGetApp().getProject()->Submit(new command::TimelineIntervalRemoveAll(getTimeline()));
}

//////////////////////////////////////////////////////////////////////////
// ACTIONS ON THE MARKED AREAS
//////////////////////////////////////////////////////////////////////////

void Intervals::deleteMarked()
{
    NIY
        // First, make one entire list containing a mapping of each clip to
        // the clips it is replaced with.

        ReplacementMap replacements;

    //BOOST_FOREACH( TrackView* track, getTimeline().mVideoTracks )
    //{
    //    ReplacementMap newreplacments = findReplacements(track);
    //    replacements.insert(newreplacments.begin(),newreplacments.end());
    //}
    //BOOST_FOREACH( TrackView* track, getTimeline().mAudioTracks )
    //{
    //    ReplacementMap newreplacments = findReplacements(track);
    //    replacements.insert(newreplacments.begin(),newreplacments.end());
    //}
    //BOOST_FOREACH(ReplacementMap::value_type entry, replacements)
    //{
    //    VAR_DEBUG(*entry.first)(*entry.second);
    //}
}

void Intervals::deleteUnmarked()
{
    NIY
}

//////////////////////////////////////////////////////////////////////////
// DRAWING
//////////////////////////////////////////////////////////////////////////

void Intervals::draw(wxDC& dc)
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
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

wxRect Intervals::makeRect(long x1, long x2)
{
    return wxRect(std::min(x1,x2),Constants::sTimeScaleHeight,std::abs(x2 - x1) + 1,Constants::sMinimalGreyAboveVideoTracksHeight);
}

wxRect Intervals::ptsToPixels(wxRect rect)
{
    rect.x = getZoom().ptsToPixels(rect.x);
    rect.width = getZoom().ptsToPixels(rect.width);
    return rect;
}

void Intervals::refresh(long begin, long end)
{
    wxRect r(ptsToPixels(makeRect(begin,end)));
    
    // Adjust for scrolling
    r.x -= getTimeline().getScrollOffset().x;
    r.y -= getTimeline().getScrollOffset().y;

    // enlargement to ensure that the vertical black end line of adjacent rects will be (re)drawn. Typical use: remove in the middle of an interval.
    r.x -= 1;
    r.width += 2;
    getTimeline().RefreshRect(r);
}

Intervals::ReplacementMap Intervals::findReplacements(TrackView* track)
{
    std::map< model::ClipPtr, model::ClipPtr > replacements;

    int pts_left = 0;
    int pts_right = 0;

    BOOST_FOREACH( model::ClipPtr modelclip, track->getTrack()->getClips() )
    {
        ClipView* clip = getViewMap().getView(modelclip);
        pts_right += clip->getClip()->getNumberOfFrames();
        wxRect cliprect = makeRect(pts_left, pts_right);
        //wxRegionContain {
        //    wxOutRegion = 0,
        //        wxPartRegion = 1,
        //        wxInRegion = 2 
        if (wxOutRegion != mMarkedIntervals.Contains(cliprect))
        {
            //clip->setSelected(true);
            wxRegion intersect(mMarkedIntervals);
            intersect.Intersect(cliprect);
            wxRegionIterator it(intersect);
            while (it)
            {
                wxRect marked = it.GetRect();
                marked.x -= pts_left;
                // todo only shows last segment
                clip->show(ptsToPixels(marked));

                model::ClipPtr original = clip->getClip();
                model::ClipPtr replacement = make_cloned<model::Clip>(original);
                replacement->setOffset(original->getOffset() + it.GetRect().GetX() - pts_left);
                replacement->setLength(it.GetRect().GetWidth());

                replacements[clip->getClip()] = replacement;

                it++;
            }

        }
        pts_left += clip->getClip()->getNumberOfFrames();
    }

    return replacements;
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

