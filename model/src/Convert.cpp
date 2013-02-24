#include "Convert.h"

#include "AudioChunk.h"
#include "Constants.h"
#include "Properties.h"
#include "UtilFrameRate.h"
#include "UtilLog.h"

namespace model {

// static
pts Convert::timeToPts(int time)
{
    return floor(rational(time) / rational(Constants::sSecond) * Properties::get().getFrameRate());
}

// static
int Convert::ptsToTime(pts position)
{
    return floor(rational(position) * rational(Constants::sSecond) / Properties::get().getFrameRate());
}

// static
int Convert::ptsToMicroseconds(pts position)
{
    return floor(rational(ptsToTime(position)) * rational(Constants::sMicroseconds));
}

// static
pts Convert::microsecondsToPts(int us)
{
    return timeToPts(floor(rational(us) / rational(Constants::sMicroseconds)));
}

// static
wxString Convert::msToHumanReadibleString(int ms)
{
    std::ostringstream o;

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
wxString Convert::ptsToHumanReadibleString(pts duration)
{
    return msToHumanReadibleString(ptsToTime(duration));
}

// static
int Convert::ptsToSamples(int audioRate, int nAudioChannels, pts position)
{
    boost::int64_t nFrames =
        static_cast<boost::int64_t>(audioRate * nAudioChannels) *
        static_cast<boost::int64_t>(model::Convert::ptsToTime(position)) /
        static_cast<boost::int64_t>(Constants::sSecond);
    ASSERT_MORE_THAN_EQUALS_ZERO(nFrames);
    return nFrames;
}

// static
pts Convert::samplesToPts(int audioRate, int nAudioChannels, int nFrames)
{
    boost::int64_t time =
        static_cast<boost::int64_t>(nFrames) *
        static_cast<boost::int64_t>(Constants::sSecond) /
        static_cast<boost::int64_t>(audioRate * nAudioChannels);
    ASSERT_MORE_THAN_EQUALS_ZERO(time);
    return model::Convert::timeToPts(time);
}

// static
int Convert::samplesToFrames(int nChannels, int nSamples)
{
    ASSERT_ZERO(nSamples % nChannels);
    return nSamples / nChannels;
}

// static
int Convert::framesToSamples(int nChannels, int nFrames)
{
    return nFrames * nChannels;
}

pts convertFrameRate(pts inputposition, FrameRate inputrate, FrameRate outputrate)
{
    return floor(rational(inputposition) / inputrate * outputrate );
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
    return audioSamplesToBytes(audioFramesToSamples(nFrames, nChannels));
}

// static
int Convert::audioSamplesToBytes(int nSamples)
{
    return nSamples * AudioChunk::sBytesPerSample;
}

} // namespace