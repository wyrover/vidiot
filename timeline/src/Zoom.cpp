#include "Zoom.h"

#include <math.h>
#include <boost/assign/list_of.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "UtilLog.h"
#include "Convert.h"
#include "Layout.h"
#include "UtilSerializeBoost.h"

namespace gui { namespace timeline {

    DEFINE_EVENT(ZOOM_CHANGE_EVENT, ZoomChangeEvent, rational);

    static rational sDefaultZoom(1,5);
typedef std::list<rational> zoomlist;
static zoomlist sZooms = boost::assign::list_of
(rational(1,1))
(rational(1,2))
(rational(1,3))
(rational(1,4))
(rational(1,5))
(rational(1,6))
(rational(1,7))
(rational(1,8))
(rational(1,9))
(rational(1,10));

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Zoom::Zoom(Timeline* timeline)
:   wxEvtHandler()
,   Part(timeline)
,   mZoom(sDefaultZoom)
{
}

Zoom::~Zoom()
{
}

void Zoom::change(int steps)
{
    rational oldzoom = mZoom;
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
    }
}

int toInt(rational r)
{
    return static_cast<int>(floor(boost::rational_cast<double>(r)));
}

int Zoom::timeToPixels(int time) const
{
    return toInt(ptsToPixels(model::Convert::timeToPts(time)));
}

int Zoom::pixelsToTime(int pixels) const
{
    return toInt(model::Convert::ptsToTime(pixelsToPts(pixels)));
}

int Zoom::pixelsToPts(int pixels) const
{
    return toInt(rational(pixels) / rational(mZoom));
}

int Zoom::ptsToPixels(int pts) const
{
    return toInt(rational(pts) * rational(mZoom));
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void Zoom::serialize(Archive & ar, const unsigned int version)
{
    ar & mZoom;
}
template void Zoom::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void Zoom::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

}} // namespace
