#include "Convert.h"

#include <math.h>
#include <boost/rational.hpp>
#include "FrameRate.h"
#include "Project.h"
#include "Constants.h"
#include "Properties.h"

namespace model {

typedef boost::rational<int> rational;

int toInt(rational r)
{
    return static_cast<int>(floor(boost::rational_cast<double>(r)));
}

// static
int Convert::timeToPts(int time)
{
    return toInt(rational(time) / rational(Constants::sSecond) / Project::current()->getProperties()->getFrameRate());
}

// static
int Convert::ptsToTime(int pts)
{
    return toInt(rational(pts) * rational(Constants::sSecond) * Project::current()->getProperties()->getFrameRate());
}

// static
int Convert::ptsToMicroseconds(int pts)
{
    return toInt(rational(ptsToTime(pts)) * rational(Constants::sMicroseconds));
}

// static
int Convert::microsecondsToPts(int us)
{
    return timeToPts(toInt(rational(us) / rational(Constants::sMicroseconds)));
}

// static
int Convert::ptsToFrames(int audioRate, int nAudioChannels, pts position)
{
    return
        audioRate *
        nAudioChannels * 
        model::Convert::ptsToTime(position) /
        Constants::sSecond;
}

} // namespace
