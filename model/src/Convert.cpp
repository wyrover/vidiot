#include "Convert.h"

#include "AudioChunk.h"
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
    return toInt(rational(time) / rational(Constants::sSecond) / Properties::get().getFrameRate());
}

// static
int Convert::ptsToTime(pts position)
{
    return toInt(rational(position) * rational(Constants::sSecond) * Properties::get().getFrameRate());
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
wxString Convert::ptsToHumanReadibleString(pts duration)
{
    std::ostringstream o;
    int ms = ptsToTime(duration);

    div_t divhours   = div(ms,              Constants::sHour);
    div_t divminutes = div(divhours.rem,    Constants::sMinute);
    div_t divseconds = div(divminutes.rem,  Constants::sSecond);

    if (divhours.quot > 0)
    {
        o << std::setw(2) << std::setfill('0') << divhours.quot << ':';
    }
    if (divminutes.quot > 0)
    {
        o << std::setw(2) << std::setfill('0') << divminutes.quot << ':';
    }
    o << std::setw(2) << std::setfill('0') << divseconds.quot << '.' << std::setw(3) << std::setfill('0') << divseconds.rem;
    return o.str();
}

// static
int Convert::ptsToFrames(int audioRate, int nAudioChannels, pts position)
{
    boost::int64_t nFrames =
        static_cast<boost::int64_t>(audioRate * nAudioChannels) *
        static_cast<boost::int64_t>(model::Convert::ptsToTime(position)) /
        static_cast<boost::int64_t>(Constants::sSecond);
    ASSERT_MORE_THAN_EQUALS_ZERO(nFrames);
    return nFrames;
}

// static
pts Convert::framesToPts(int audioRate, int nAudioChannels, int nFrames)
{
    boost::int64_t time =
        static_cast<boost::int64_t>(nFrames) *
        static_cast<boost::int64_t>(Constants::sSecond) /
        static_cast<boost::int64_t>(audioRate * nAudioChannels);
    ASSERT_MORE_THAN_EQUALS_ZERO(time);
    return model::Convert::timeToPts(time);
}

pts convertFrameRate(pts inputposition, FrameRate inputrate, FrameRate outputrate)
{
    return toInt(rational(inputposition) * inputrate / outputrate );
}

//static
pts Convert::toProjectFrameRate(pts inputposition, FrameRate inputrate)
{
    return convertFrameRate(inputposition, inputrate, Properties::get().getFrameRate());
}

//static
pts Convert::fromProjectFrameRate(pts outputposition, FrameRate inputrate)
{
    return convertFrameRate(outputposition, Properties::get().getFrameRate(), inputrate);
}

// static
int Convert::scale(int input, double factor)
{
    return static_cast<int>(floor(factor * input));
}

// static
wxSize Convert::scale(wxSize input, double factor)
{
    return wxSize(scale(input.x,factor),scale(input.y,factor));
}

// static
wxPoint Convert::scale(wxPoint input, double factor)
{
    return wxPoint(scale(input.x,factor),scale(input.y,factor));
}

// static
wxRect Convert::scale(wxRect input, double factor)
{
    return wxRect(scale(input.GetPosition(),factor),scale(input.GetSize(),factor));
}

// static
wxSize Convert::sizeInBoundingBox(wxSize input, wxSize boundingbox, double& scaling)
{
    static const int sMinimumSize = 10; // Used to avoid crashes in sws_scale (too small bitmaps)
    double w = std::max(sMinimumSize, boundingbox.GetWidth());
    double h = std::max(sMinimumSize, boundingbox.GetHeight());
    double scalingW = w / static_cast<double>(input.GetWidth());
    double scalingH = h / static_cast<double>(input.GetHeight());
    scaling  = std::min(scalingW, scalingH);
    return scale(input,scaling);
}

// static
wxSize Convert::sizeInBoundingBox(wxSize input, wxSize boundingbox)
{
    double dummy;
    return sizeInBoundingBox(input,boundingbox,dummy);
}

// static
wxSize Convert::fillBoundingBoxWithMinimalLoss(wxSize input, wxSize boundingbox, double& scaling)
{
    static const int sMinimumSize = 10; // Used to avoid crashes in sws_scale (too small bitmaps)
    double w = std::max(sMinimumSize, boundingbox.GetWidth());
    double h = std::max(sMinimumSize, boundingbox.GetHeight());
    double scalingW = w / static_cast<double>(input.GetWidth());
    double scalingH = h / static_cast<double>(input.GetHeight());
    scaling  = std::max(scalingW, scalingH);
    return scale(input,scaling);
}

// static
int Convert::doubleToInt(double x)
{
    return (x >= 0.0) ? static_cast<int>(std::floor(x + 0.5)) : static_cast<int>(std::ceil(x - 0.5));
}

// static
int Convert::factorToDigits(double number, int nDigits)
{
    double digitfactor = pow(static_cast<float>(10),nDigits);
    return doubleToInt(number * digitfactor);
}

// static
double Convert::digitsToFactor(int number, int nDigits)
{
    double digitfactor = pow(static_cast<float>(10), nDigits);
    return static_cast<double>(number) / digitfactor;
}

// static
int Convert::audioFramesToSamples(int nFrames, int nChannels)
{
    return nFrames * nChannels;
}

// static
int Convert::audioFramesToBytes(int nFrames, int nChannels)
{
    return audioFramesToSamples(nFrames, nChannels) * AudioChunk::sBytesPerSample;
}

} // namespace