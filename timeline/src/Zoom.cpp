// Copyright 2013-2016 Eric Raijmakers.
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
#include "ProjectModification.h"
#include "Scrolling.h"
#include "UtilSerializeBoost.h"

namespace gui { namespace timeline {

static rational64 sDefaultZoom{ 1,5 };
typedef std::vector<rational64> zoomlist;
// NOTE: Match with map used in TimescaleView!!!
static zoomlist sZooms = {
    { rational64{1, 120} },
    { rational64{1, 60} },
    { rational64{1, 45} },
    { rational64{1, 30} },
    { rational64{1, 20} },
    { rational64{1, 15} },
    { rational64{1, 10} },
    { rational64{1, 9} },
    { rational64{1, 8} },
    { rational64{1, 7} },
    { rational64{1, 6} },
    { rational64{1, 5} },
    { rational64{1, 4} },
    { rational64{1, 3} },
    { rational64{1, 2} },
    { rational64{1, 1} },
    { rational64{2, 1} },
};

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Zoom::Zoom(Timeline* timeline)
:   Part(timeline)
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

rational64 Zoom::getCurrent() const
{
    return mZoom;
}

void Zoom::change(int steps)
{
    getScrolling().storeCenterPts();
    rational64 oldzoom = mZoom;
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
        model::ProjectModification::trigger();
        getTimeline().beginTransaction();
        getViewMap().invalidateClipPreviews();
        getTimeline().resize();
        getScrolling().alignCenterPts();
        if (getDrag().isActive())
        {
            getDrag().show(); // Use new bitmap
        }
        getTimeline().endTransaction();
        getTimeline().Refresh(false);
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
    return floor(rational64{ pixels } / rational64{ mZoom });
}

int Zoom::ptsToPixels(pts position) const
{
    return ptsToPixels(position, mZoom);
}

// static 
int Zoom::ptsToPixels(pts position, rational64 zoom)
{
    return floor(rational64{ position } *rational64{ zoom });
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Zoom::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        if (version < 2)
        {
            rational32 zoom;
            ar & boost::serialization::make_nvp("mZoom", zoom);
            mZoom = rational64{ zoom.numerator(), zoom.denominator() };;
        }
        else
        {
            ar & BOOST_SERIALIZATION_NVP(mZoom);
        }
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void Zoom::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void Zoom::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

}} // namespace