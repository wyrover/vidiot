#include "Convert.h"

#include <math.h>
#include <boost/rational.hpp>
#include "Project.h"
#include "Constants.h"
#include "UtilLog.h"
#include "Properties.h"

namespace model {

typedef boost::rational<int> rational;

int toInt(rational r)
{
    return static_cast<int>(floor(boost::rational_cast<double>(r)));
}

// static
pts Convert::timeToPts(int time)
{
    return toInt(rational(time) / rational(Constants::sSecond) / Project::get().getProperties()->getFrameRate());
}

// static
int Convert::ptsToTime(pts position)
{
    return toInt(rational(position) * rational(Constants::sSecond) * Project::get().getProperties()->getFrameRate());
}

// static
int Convert::ptsToMicroseconds(pts position)
{
    return toInt(rational(ptsToTime(position)) * rational(Constants::sMicroseconds));
}

// static
pts Convert::microsecondsToPts(int us)
{
    return timeToPts(toInt(rational(us) / rational(Constants::sMicroseconds)));
}

// static
int Convert::ptsToFrames(int audioRate, int nAudioChannels, pts position)
{
    boost::int64_t nFrames =
        static_cast<boost::int64_t>(audioRate * nAudioChannels) * 
        static_cast<boost::int64_t>(model::Convert::ptsToTime(position)) /
        static_cast<boost::int64_t>(Constants::sSecond);
    ASSERT(nFrames >= 0);
    return nFrames;
}

// static
pts Convert::framesToPts(int audioRate, int nAudioChannels, int nFrames)
{
    boost::int64_t time = 
        static_cast<boost::int64_t>(nFrames) * 
        static_cast<boost::int64_t>(Constants::sSecond) /
        static_cast<boost::int64_t>(audioRate * nAudioChannels);
    ASSERT(time >= 0);
    return model::Convert::timeToPts(time); 
}

pts convertFrameRate(pts inputposition, FrameRate inputrate, FrameRate outputrate)
{
    return toInt(rational(inputposition) * inputrate / outputrate );
}

//static 
pts Convert::toProjectFrameRate(pts inputposition, FrameRate inputrate)
{
    return convertFrameRate(inputposition, inputrate, Project::get().getProperties()->getFrameRate());
}

//static 
pts Convert::fromProjectFrameRate(pts outputposition, FrameRate inputrate)
{
    return convertFrameRate(outputposition, Project::get().getProperties()->getFrameRate(), inputrate);
}

// static 
wxSize Convert::sizeInBoundingBox(wxSize input, wxSize boundingbox)
{
    static const int sMinimumSize = 10; // Used to avoid crashes in sws_scale (too small bitmaps)
    double w = std::max(sMinimumSize, boundingbox.GetWidth());
    double h = std::max(sMinimumSize, boundingbox.GetHeight());
    double scalingW = w / static_cast<double>(input.GetWidth());
    double scalingH = h / static_cast<double>(input.GetHeight());
    double scaling  = std::min(scalingW, scalingH);
    int scaledWidth  = static_cast<int>(floor(scaling * input.GetWidth()));
    int scaledHeight = static_cast<int>(floor(scaling * input.GetHeight()));
    return wxSize(scaledWidth,scaledHeight);
}

} // namespace
