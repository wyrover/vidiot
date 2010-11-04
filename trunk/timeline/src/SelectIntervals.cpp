#include "SelectIntervals.h"

#include <math.h>
#include <boost/foreach.hpp>
#include <boost/make_shared.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "GuiTimeLine.h"
#include "GuiTimeLineZoom.h"
#include "UtilLog.h"
#include "Constants.h"
#include "GuiOptions.h"
#include "GuiMain.h"
#include "GuiTimeLineTrack.h"
#include "GuiTimeLineClip.h"
#include "Project.h"
#include "TimelineIntervalChange.h"
#include "TimelineIntervalRemoveAll.h"
#include "UtilSerializeWxwidgets.h"
#include "ids.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

SelectIntervals::SelectIntervals()
:   wxEvtHandler()
,	mTimeline(0)
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

    wxGetApp().GetTopWindow()->Bind(wxEVT_COMMAND_MENU_SELECTED,  &SelectIntervals::OnDeleteMarked,   this, ID_DELETEMARKED);
    wxGetApp().GetTopWindow()->Bind(wxEVT_COMMAND_MENU_SELECTED,  &SelectIntervals::OnDeleteUnmarked, this, ID_DELETEUNMARKED);
    wxGetApp().GetTopWindow()->Bind(wxEVT_COMMAND_MENU_SELECTED,  &SelectIntervals::OnRemoveMarkers,  this, ID_REMOVEMARKERS);

    updateMenu();
}

//////////////////////////////////////////////////////////////////////////
// MARKING / TOGGLING INTERFACE
//////////////////////////////////////////////////////////////////////////

wxRegion SelectIntervals::get()
{
    return mMarkedIntervals;
}

void SelectIntervals::set(wxRegion region)
{
    wxRect r = mMarkedIntervals.GetBox().Union(region.GetBox());
    mMarkedIntervals = region;
    updateMenu();
    refresh(r.x,r.x + r.width);
}

void SelectIntervals::addBeginMarker()
{
    long c = mTimeline->mZoom.pixelsToPts(mTimeline->mCursorPosition);
    long b = c + mTimeline->mZoom.timeToPts(GuiOptions::getMarkerBeginAddition() * Constants::sSecond);
    long e = c + mTimeline->mZoom.timeToPts(GuiOptions::getMarkerEndAddition()   * Constants::sSecond);

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
    mToggleBegin = mTimeline->mZoom.pixelsToPts(mTimeline->mCursorPosition);
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
    updateMenu();
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
        mNewIntervalEnd = mTimeline->mZoom.pixelsToPts(event.getValue()) +  mTimeline->mZoom.timeToPts(GuiOptions::getMarkerEndAddition() * Constants::sSecond);
        refresh(mNewIntervalBegin,mNewIntervalEnd);
    }
    if (mToggleActive)
    {
        mToggleEnd = mTimeline->mZoom.pixelsToPts(event.getValue());
        refresh(mToggleBegin,mToggleEnd);
    }
}

SelectIntervals::ReplacementMap SelectIntervals::findReplacements(GuiTimeLineTrackPtr track)
{
    std::map< model::ClipPtr, model::ClipPtr > replacements;

    int pts_left = 0;
    int pts_right = 0;

    BOOST_FOREACH( model::ClipPtr modelclip, track->getTrack()->getClips() )
    {
        GuiTimeLineClipPtr clip = mTimeline->getViewMap().ModelToView(modelclip);
        pts_right += clip->getClip()->getNumberOfFrames();
        wxRect cliprect = makeRect(pts_left, pts_right);
        //wxRegionContain {
        //    wxOutRegion = 0,
        //        wxPartRegion = 1,
        //        wxInRegion = 2 
        if (wxOutRegion != mMarkedIntervals.Contains(cliprect))
        {
            clip->setSelected(true);
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

void SelectIntervals::OnDeleteMarked(wxCommandEvent& WXUNUSED(event))
{
    // First, make one entire list containing a mapping of each clip to
    // the clips it is replaced with.

    ReplacementMap replacements;

    BOOST_FOREACH( GuiTimeLineTrackPtr track, mTimeline->mVideoTracks )
    {
        ReplacementMap newreplacments = findReplacements(track);
        replacements.insert(newreplacments.begin(),newreplacments.end());
    }
    BOOST_FOREACH( GuiTimeLineTrackPtr track, mTimeline->mAudioTracks )
    {
        ReplacementMap newreplacments = findReplacements(track);
        replacements.insert(newreplacments.begin(),newreplacments.end());
    }
    BOOST_FOREACH(ReplacementMap::value_type entry, replacements)
    {
        VAR_DEBUG(*entry.first)(*entry.second);
    }
}

void SelectIntervals::OnDeleteUnmarked(wxCommandEvent& WXUNUSED(event))
{
    NIY
}

void SelectIntervals::OnRemoveMarkers(wxCommandEvent& WXUNUSED(event))
{
    wxGetApp().getProject()->Submit(new command::TimelineIntervalRemoveAll(shared_from_this()));
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
    rect.x = mTimeline->mZoom.ptsToPixels(rect.x);
    rect.width = mTimeline->mZoom.ptsToPixels(rect.width);
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

void SelectIntervals::updateMenu()
{
    mTimeline->mMenu.Enable( ID_DELETEMARKED,   !mMarkedIntervals.IsEmpty() );
    mTimeline->mMenu.Enable( ID_DELETEUNMARKED, !mMarkedIntervals.IsEmpty() );
    mTimeline->mMenu.Enable( ID_REMOVEMARKERS,  !mMarkedIntervals.IsEmpty() );
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

}} // namespace

