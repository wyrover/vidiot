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

#include "Zoom.h"

#include "Convert.h"
#include "Cursor.h"
#include "Layout.h"
#include "ProjectModification.h"
#include "Scrolling.h"
#include "UtilInt.h"
#include "UtilLog.h"
#include "UtilSerializeBoost.h"

namespace gui { namespace timeline {

DEFINE_EVENT(ZOOM_CHANGE_EVENT, ZoomChangeEvent, rational);

static rational sDefaultZoom(1,5);
typedef std::list<rational> zoomlist;
static zoomlist sZooms = boost::assign::list_of // NOTE: Match with map used in TimescaleView!!!
(rational(1,120))
(rational(1,60))
(rational(1,45))
(rational(1,30))
(rational(1,20))
(rational(1,15))
(rational(1,10))
(rational(1,9))
(rational(1,8))
(rational(1,7))
(rational(1,6))
(rational(1,5))
(rational(1,4))
(rational(1,3))
(rational(1,2))
(rational(1,1));

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Zoom::Zoom(Timeline* timeline)
:   wxEvtHandler()
,   Part(timeline)
,   mZoom(sDefaultZoom)
{
    VAR_DEBUG(this);
}

Zoom::~Zoom()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// ZOOM
//////////////////////////////////////////////////////////////////////////

rational Zoom::getCurrent() const
{
    return mZoom;
}

void Zoom::change(int steps)
{
    getScrolling().storeCenterPts();
    rational oldzoom = mZoom;
    pts cursorPosition = getCursor().getLogicalPosition();
    zoomlist::iterator it = find(sZooms.begin(), sZooms.end(), mZoom);
    while (steps > 0)
    {
        ++it;
        if (it == sZooms.end()) break;
        mZoom = *it;
        --steps;
    }
    while (steps < 0)
    {
        if (it == sZooms.begin()) break;
        --it;
        mZoom = *it;
        ++steps;
    }
    if (oldzoom != mZoom)
    {
        VAR_INFO(mZoom);
        QueueEvent(new ZoomChangeEvent(mZoom));
        model::ProjectModification::trigger();
        getCursor().setLogicalPosition(cursorPosition);
    }
}

int Zoom::timeToPixels(int time) const
{
    return ptsToPixels(model::Convert::timeToPts(time));
}

int Zoom::pixelsToTime(int pixels) const
{
    return model::Convert::ptsToTime(pixelsToPts(pixels));
}

pts Zoom::pixelsToPts(int pixels) const
{
    return floor(rational(pixels) / rational(mZoom));
}

int Zoom::ptsToPixels(pts position) const
{
    return floor(rational(position) * rational(mZoom));
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Zoom::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & mZoom;
    }
    catch (boost::archive::archive_exception& e) { VAR_ERROR(e.what());                         throw; }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void Zoom::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Zoom::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

}} // namespace