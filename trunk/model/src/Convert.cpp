// Copyright 2013,2014 Eric Raijmakers.
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

#include "Convert.h"

#include <boost/math/constants/constants.hpp>
#include "AudioChunk.h"
#include "Constants.h"
#include "Properties.h"
#include "UtilFrameRate.h"
#include "UtilLog.h"

namespace model {

// static
pts Convert::timeToPts(const milliseconds& time)
{
    return rationaltimeToPts(rational64(time));
}

// static
pts Convert::rationaltimeToPts(const rational64& time)
{
    return rationaltimeToPts(time, Properties::get().getFrameRate());
}

// static
pts Convert::timeToPts(const milliseconds& time, const FrameRate& framerate)
{
    return rationaltimeToPts(rational64(time), framerate);
}

// static
pts Convert::rationaltimeToPts(const rational64& time, const FrameRate& framerate )
{
    return floor64(time / rational64(Constants::sSecond) * framerate );
}

// static
milliseconds Convert::ptsToTime(const pts& position)
{
    return floor64(rational64(position) * rational64(Constants::sSecond) / Properties::get().getFrameRate());
}

// static
microseconds Convert::ptsToMicroseconds(const pts& position)
{
    return floor64(rational64(ptsToTime(position)) * rational64(Constants::sMicroseconds));
}

// static
pts Convert::microsecondsToPts(const microseconds& us)
{
    return timeToPts(floor(rational(us) / rational(Constants::sMicroseconds)));
}

// static
wxString Convert::msToHumanReadibleString(const milliseconds& ms)
{
    std::ostringstream o;

    lldiv_t divhours   = lldiv(ms,              Constants::sHour);
    lldiv_t divminutes = lldiv(divhours.rem,    Constants::sMinute);
    lldiv_t divseconds = lldiv(divminutes.rem,  Constants::sSecond);

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
wxString Convert::ptsToHumanReadibleString(const pts& duration)
{
    return msToHumanReadibleString(ptsToTime(duration));
}

// static
samplecount Convert::ptsToSamples(const int& audioRate, const int& nAudioChannels, const pts& position)
{
    int64_t nFrames =
        removeRemainder(nAudioChannels, // Ensure that the returned value is never aligned such that the data for one or more speakers is missing
        static_cast<int64_t>(audioRate * nAudioChannels) *
        static_cast<int64_t>(model::Convert::ptsToTime(position)) /
        static_cast<int64_t>(Constants::sSecond));
    ASSERT_MORE_THAN_EQUALS_ZERO(nFrames);
    return nFrames;
}

// static
pts Convert::samplesToPts(const int& audioRate, const int& nAudioChannels, const samplecount& nSamples)
{
    int64_t time =
        static_cast<int64_t>(nSamples) *
        static_cast<int64_t>(Constants::sSecond) /
        static_cast<int64_t>(audioRate * nAudioChannels);
    ASSERT_MORE_THAN_EQUALS_ZERO(time);
    return model::Convert::timeToPts(time);
}

// static
samplecount Convert::samplesToFrames(const int& nChannels, const samplecount& nSamples)
{
    ASSERT_ZERO(nSamples % nChannels);
    return nSamples / nChannels;
}

// static
samplecount Convert::framesToSamples(const int& nChannels, const samplecount& nFrames)
{
    return nFrames * nChannels;
}

pts convertFrameRate(const pts& inputposition, const FrameRate& inputrate, const FrameRate& outputrate)
{
    return floor64(rational64(inputposition) / inputrate * outputrate );
}

//static
pts Convert::toProjectFrameRate(const pts& inputposition, const FrameRate& inputrate)
{
    return convertFrameRate(inputposition, inputrate, Properties::get().getFrameRate());
}

//static
pts Convert::fromProjectFrameRate(const pts& outputposition, const FrameRate& inputrate)
{
    return convertFrameRate(outputposition, Properties::get().getFrameRate(), inputrate);
}

// static
int Convert::scale(const int& input, const boost::rational<int>& factor)
{
    return floor(factor * boost::rational<int>(input));
}

// static
wxSize Convert::scale(const wxSize& input, const boost::rational<int>& factor)
{
    return wxSize(scale(input.x,factor),scale(input.y,factor));
}

// static
wxPoint Convert::scale(const wxPoint& input, const boost::rational<int>& factor)
{
    return wxPoint(scale(input.x,factor),scale(input.y,factor));
}

// static
wxRect Convert::scale(const wxRect& input, const boost::rational<int>& factor)
{
    return wxRect(scale(input.GetPosition(),factor),scale(input.GetSize(),factor));
}

// static
wxSize Convert::sizeInBoundingBox(const wxSize& input, const wxSize& boundingbox, boost::rational<int>& scaling, bool fill)
{
    boost::rational<int> bbWidth(boundingbox.GetWidth());
    boost::rational<int> inWidth(input.GetWidth());
    boost::rational<int> scWidth = bbWidth / inWidth;

    boost::rational<int> bbHeight(boundingbox.GetHeight());
    boost::rational<int> inHeight(input.GetHeight());
    boost::rational<int> scHeight = bbHeight / inHeight;

    ASSERT_LESS_THAN_EQUALS(scWidth  * boost::rational<int>(input.GetWidth()),   boost::rational<int>(boundingbox.GetWidth()));
    ASSERT_LESS_THAN_EQUALS(scHeight * boost::rational<int>(input.GetHeight()), boost::rational<int>(boundingbox.GetHeight()));

    if (fill)
    {
        scaling = std::max(scWidth, scHeight);
    }
    else
    {
        scaling = std::min(scWidth, scHeight);
    }

    return scale(input,scaling);
}

// static
wxSize Convert::sizeInBoundingBox(const wxSize& input, const wxSize& boundingbox)
{
    boost::rational<int> dummy;
    return sizeInBoundingBox(input,boundingbox,dummy);
}

// static
double Convert::degreesToRadians(const boost::rational<int>& degrees) // todo more const& throughout code
{
    return -1 * boost::rational_cast<double>(degrees) * boost::math::constants::pi<double>() / 180.0;
}

// static
int Convert::doubleToInt(const double& x)
{
    return (x >= 0.0) ? static_cast<int>(std::floor(x + 0.5)) : static_cast<int>(std::ceil(x - 0.5));
}

// static
samplecount Convert::audioFramesToSamples(const samplecount& nFrames, const int& nChannels)
{
    return nFrames * nChannels;
}

// static
samplecount Convert::audioFramesToBytes(const samplecount& nFrames, const int& nChannels)
{
    return audioSamplesToBytes(audioFramesToSamples(nFrames, nChannels));
}

// static
samplecount Convert::audioSamplesToBytes(const samplecount& nSamples)
{
    return nSamples * AudioChunk::sBytesPerSample;
}

} // namespace