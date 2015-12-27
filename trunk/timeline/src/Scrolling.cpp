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

#include "Scrolling.h"

#include "Cursor.h"
#include "Mouse.h"
#include "Timeline.h"
#include "Zoom.h"

namespace gui { namespace timeline {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Scrolling::Scrolling(Timeline* timeline)
:   Part(timeline)
,   mCenterPts(0)
,   mRightScrollOrigin(0,0)
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
// GUI EVENTS
//////////////////////////////////////////////////////////////////////////

void Scrolling::rightDown()
{
    mRightScrollOrigin = getMouse().getPhysicalPosition();
}

void Scrolling::update(state::EvMouse& state)
{
    wxPoint current = state.Position;

    int dx = mRightScrollOrigin.x - current.x;
    int dy = mRightScrollOrigin.y - current.y;

    if (state.RightIsDown && (dx != 0 || dy !=0))
    {
        rational64 factor_x{ getTimeline().GetVirtualSize().x, getTimeline().GetClientSize().x };
        factor_x = std::max(factor_x, rational64{ 1 }); // Factor >= 1
        rational64 factor_y{ getTimeline().GetVirtualSize().y, getTimeline().GetClientSize().y };
        factor_y = std::max(factor_y, rational64{ 1 }); // Factor >= 1

        int x ,y;
        getTimeline().GetViewStart(&x,&y);
        x += floor(rational64{ dx } *factor_x);
        x = std::max(x,0); // x >= 0
        y += floor(rational64{ dy } *factor_y);
        y = std::max(y,0); // y >= 0
        getTimeline().Scroll(x, y);
    }
    mRightScrollOrigin = current;
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

pts Scrolling::getFourthPts() const
{
    wxPoint position = getOffset();
    wxSize size = getTimeline().GetClientSize();
    return getZoom().pixelsToPts( position.x + size.x / 4 );
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
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void Scrolling::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void Scrolling::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

}} // namespace
