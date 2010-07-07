#include "GuiTimeLineZoom.h"
#include "GuiOptions.h"
#include "FrameRate.h"
#include "Project.h"
#include "UtilLog.h"
#include <math.h>
#include "Constants.h"
#include "Properties.h"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "UtilSerializeBoost.h"

static rational sDefaultZoom(1,5);

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION METHODS
//////////////////////////////////////////////////////////////////////////

GuiTimeLineZoom::GuiTimeLineZoom()
:   mZoom(sDefaultZoom)
{
}

GuiTimeLineZoom::~GuiTimeLineZoom()
{
}

int toInt(rational r)
{
    return static_cast<int>(floor(boost::rational_cast<double>(r)));
}

int GuiTimeLineZoom::timeToPixels(int time) const
{
    return toInt(ptsToPixels(timeToPts(time)));
}

int GuiTimeLineZoom::pixelsToTime(int pixels) const
{
    return toInt(ptsToTime(pixelsToPts(pixels)));
}

int GuiTimeLineZoom::pixelsToPts(int pixels) const
{
    return toInt(rational(pixels) / rational(mZoom));
}

int GuiTimeLineZoom::ptsToPixels(int pts) const
{
    return toInt(rational(pts) * rational(mZoom));
}

// static
int GuiTimeLineZoom::timeToPts(int time)
{
    return toInt(rational(time) / rational(Constants::sSecond) / model::Project::current()->getProperties()->getFrameRate());
}

// static
int GuiTimeLineZoom::ptsToTime(int pts)
{
    return toInt(rational(pts) * rational(Constants::sSecond) * model::Project::current()->getProperties()->getFrameRate());
}

// static
int GuiTimeLineZoom::ptsToMicroseconds(int pts)
{
    return toInt(rational(ptsToTime(pts)) * rational(Constants::sMicrosecondsPerSecond));
}

// static
int GuiTimeLineZoom::microsecondsToPts(int us)
{
    return timeToPts(toInt(rational(us) / rational(Constants::sMicrosecondsPerSecond)));
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION 
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void GuiTimeLineZoom::serialize(Archive & ar, const unsigned int version)
{
    ar & mZoom;
}
template void GuiTimeLineZoom::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void GuiTimeLineZoom::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);


