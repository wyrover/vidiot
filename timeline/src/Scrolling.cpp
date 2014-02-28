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

#include "Scrolling.h"

#include "Cursor.h"
#include "Timeline.h"
#include "UtilLog.h"
#include "Zoom.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Scrolling::Scrolling(Timeline* timeline)
:   Part(timeline)
,   mCenterPts(0)
{
    VAR_DEBUG(this);

    getTimeline().SetScrollRate( 1, 1 );
    getTimeline().EnableScrolling(true,true);
}

Scrolling::~Scrolling()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

wxPoint Scrolling::getOffset() const
{
    int scrollX, scrollY, ppuX, ppuY;
    getTimeline().GetViewStart(&scrollX,&scrollY);
    getTimeline().GetScrollPixelsPerUnit(&ppuX,&ppuY);
    ASSERT_EQUALS(ppuX,1); // Other values not allowed: see Scrolling::align()
    ASSERT_EQUALS(ppuY,1); // Other values not allowed: see Scrolling::align()
    return wxPoint(scrollX * ppuX, scrollY * ppuY);
}

pixel Scrolling::align(pts position, pixel physicalPosition)
{
    pixel diff = ptsToPixel(position) - physicalPosition;
    pixel remaining = 0;
    if (diff != 0)
    {
        int xOld;
        int y;
        getTimeline().GetViewStart(&xOld,&y);
        int xNew = xOld + diff;
        if (xNew < 0)
        {
            remaining = xNew; // return the remaining 'to be scrolled'
            xNew = 0; // Scroll as far as possible
        }
        if (xNew != xOld)
        {
            getTimeline().Scroll(xNew, -1);
        }

    }
    return remaining;
}

pixel Scrolling::ptsToPixel(pts position) const
{
    return getTimeline().CalcScrolledPosition(wxPoint(getZoom().ptsToPixels(position),0)).x;
}

wxPoint Scrolling::getVirtualPosition(const wxPoint& position) const
{
    wxPoint p;
    getTimeline().CalcUnscrolledPosition(position.x,position.y,&p.x,&p.y);
    return p;
}

void Scrolling::storeCenterPts()
{
    mCenterPts = getCenterPts();
}

pts Scrolling::getCenterPts() const
{
    wxPoint position = getOffset();
    wxSize size = getTimeline().GetClientSize();
    if (position.x != 0)
    {
        // Store the pts value that is visible in the center of the timeline
        // After the zoom has been changed, this can be used to reposition
        // the scrollbars such that the same pts value is in the center of
        // the timeline.
        return getZoom().pixelsToPts( position.x + size.x / 2 );
    }
    // When no scrolling has been applied, the view remains 'the beginning of the timeline'
    return 0;
}

//////////////////////////////////////////////////////////////////////////
// HELPER METHODS
//////////////////////////////////////////////////////////////////////////

void Scrolling::alignCenterPts()
{
    wxSize size = getTimeline().GetClientSize();
    align(mCenterPts, size.x / 2);
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Scrolling::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        if (Archive::is_saving::value)
        {
            storeCenterPts();
        }
        ar & BOOST_SERIALIZATION_NVP(mCenterPts);
        if (Archive::is_loading::value)
        {
            alignCenterPts();
        }
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void Scrolling::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void Scrolling::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

}} // namespace