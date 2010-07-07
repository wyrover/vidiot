#include "SelectIntervals.h"

#include <boost/foreach.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "GuiTimeLine.h"
#include "GuiTimeLineZoom.h"
#include "UtilLog.h"
#include "Constants.h"
#include "GuiOptions.h"
#include <math.h>
#include <algorithm>

SelectIntervals::SelectIntervals()
{
}

SelectIntervals::~SelectIntervals()
{
}

void SelectIntervals::init(GuiTimeLinePtr timeline)
{
    mTimeline = timeline;
}

void SelectIntervals::addBeginMarker()
{
    long marker = mTimeline->mCursorPosition + mTimeline->mZoom->timeToPixels(GuiOptions::getMarkerBeginAddition() * Constants::sSecond);

    mMarkerPositions.push_back(marker);

    // Refresh the area for the new marker. This is needed due to the 'addition' 
    long cursorOnClientArea = mTimeline->mCursorPosition - mTimeline->getScrollOffset().x;
    long beginMarkerOnClientArea = marker - mTimeline->getScrollOffset().x;
    mTimeline->RefreshRect(wxRect(std::min(cursorOnClientArea,beginMarkerOnClientArea),Constants::sTimeScaleHeight,std::abs(cursorOnClientArea-beginMarkerOnClientArea)+1,Constants::sMinimalGreyAboveVideoTracksHeight),false);
}

void SelectIntervals::addEndMarker()
{
    long marker = mTimeline->mCursorPosition + mTimeline->mZoom->timeToPixels(GuiOptions::getMarkerEndAddition() * Constants::sSecond);

    mMarkerPositions.push_back(marker);

    // Refresh the area for the new marker. This is needed due to the 'addition' 
    long cursorOnClientArea = mTimeline->mCursorPosition - mTimeline->getScrollOffset().x;
    long endMarkerOnClientArea = marker - mTimeline->getScrollOffset().x;
    mTimeline->RefreshRect(wxRect(std::min(cursorOnClientArea,endMarkerOnClientArea),Constants::sTimeScaleHeight,std::abs(cursorOnClientArea-endMarkerOnClientArea)+1,Constants::sMinimalGreyAboveVideoTracksHeight),false);
}

void SelectIntervals::draw(wxDC& dc)
{
    // Draw marked areas
    dc.SetPen(wxPen(*wxGREEN, 1));
    dc.SetBrush(*wxGREEN_BRUSH);
    MarkerPositions::iterator it = mMarkerPositions.begin();
    while (it != mMarkerPositions.end())
    {
        long beginpoint = *it;
        long endpoint = 0;
        ++it;
        if (it != mMarkerPositions.end())
        {
            endpoint = *it;
            ++it;
        }
        else
        {
            // This branch handles a 'running endpoint' when holding down the
            // marker key for a while.
            // The 'max' handling is needed since expansion/contraction
            // can cause the beginpoint to be beyond the cursor position.
            long marker = mTimeline->mCursorPosition + mTimeline->mZoom->timeToPixels(GuiOptions::getMarkerEndAddition() * Constants::sSecond);
            endpoint = std::max(beginpoint,marker);

            // Refresh the area for the new marker.
            long beginOnClientArea = beginpoint - mTimeline->getScrollOffset().x;
            long endOnClientArea = endpoint - mTimeline->getScrollOffset().x;
            mTimeline->RefreshRect(wxRect(std::min(beginOnClientArea,endOnClientArea),Constants::sTimeScaleHeight,std::abs(beginOnClientArea-endOnClientArea)+1,Constants::sMinimalGreyAboveVideoTracksHeight),false);

        }
        ASSERT(endpoint >= beginpoint)(beginpoint)(endpoint);
        dc.DrawRectangle(wxRect(beginpoint, Constants::sTimeScaleHeight, endpoint-beginpoint,Constants::sMinimalGreyAboveVideoTracksHeight));
    }
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void SelectIntervals::serialize(Archive & ar, const unsigned int version)
{
    ar & mMarkerPositions;
    if (Archive::is_loading::value)
    {
        if (mMarkerPositions.size() % 2 != 0)
        {
            // This selection was incomplete. Remove it.
            // Todo: better handling. Ensure that always complete marker couples are written to file.
            mMarkerPositions.pop_back();
        }
    }
}
template void SelectIntervals::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void SelectIntervals::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);


