#include "Zoom.h"

#include <math.h>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "UtilLog.h"
#include "Convert.h"
#include "Layout.h"
#include "UtilSerializeBoost.h"

namespace gui { namespace timeline {

static rational sDefaultZoom(1,5);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

Zoom::Zoom(Timeline* timeline)
:   Part(timeline)
,   mZoom(sDefaultZoom)
{
}

Zoom::~Zoom()
{
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
