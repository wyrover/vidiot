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

#include "Convert.h"

#include <boost/math/constants/constants.hpp>
#include "AudioChunk.h"
#include "Properties.h"
#include "UtilFrameRate.h"

namespace model {

constexpr int sMicroseconds = 1000;

// static
pts Convert::timeToPts(milliseconds time)
{
    return rationaltimeToPts(rational64(time));
}

// static
pts Convert::rationaltimeToPts(rational64 time)
{
    return rationaltimeToPts(time, Properties::get().getFrameRate());
}

// static
pts Convert::timeToPts(milliseconds time, const FrameRate& framerate)
{
    return rationaltimeToPts(rational64(time), framerate);
}

// static
pts Convert::rationaltimeToPts(rational64 time, const FrameRate& framerate )
{
    return floor(time / rational64(sSecond) * framerate );
}

// static 
double Convert::timeToSeconds(milliseconds time)
{
    return boost::rational_cast<double>(rational64(time,1000));
}

// static
milliseconds Convert::ptsToTime(pts position)
{
    return ptsToTime(position, Properties::get().getFrameRate());
}

// static
double Convert::ptsToSeconds(pts position)
{
    return timeToSeconds(ptsToTime(position));
}

// static
milliseconds Convert::ptsToTime(pts position, const FrameRate& framerate)
{
    return floor(rational64(position) * rational64(sSecond) / framerate);
}

// static
microseconds Convert::ptsToMicroseconds(pts position)
{
    return floor(rational64(ptsToTime(position)) * rational64(sMicroseconds));
}

// static
pts Convert::microsecondsToPts(microseconds us)
{
    return timeToPts(floor(rational64(us) / rational64(sMicroseconds)));
}

// static
wxString Convert::msToHumanReadibleString(milliseconds ms, bool minutesAlways, bool hoursAlways)
{
    std::ostringstream o;

    lldiv_t divhours   = lldiv(ms,              sHour);
    lldiv_t divminutes = lldiv(divhours.rem,    sMinute);
    lldiv_t divseconds = lldiv(divminutes.rem,  sSecond);

    if (divhours.quot > 0 || hoursAlways)
    {
        o << std::setw(2) << std::setfill('0') << divhours.quot << ':';
    }
    if (divminutes.quot > 0 || minutesAlways)
    {
        o << std::setw(2) << std::setfill('0') << divminutes.quot << ':';
    }
    o << std::setw(2) << std::setfill('0') << divseconds.quot << '.' << std::setw(3) << std::setfill('0') << divseconds.rem;
    return o.str();
}

// static
wxString Convert::ptsToHumanReadibleString(pts duration, bool minutesAlways, bool hoursAlways)
{
    return msToHumanReadibleString(ptsToTime(duration), minutesAlways, hoursAlways);
}

// static
samplecount Convert::ptsToSamplesPerChannel(int audioRate, pts position)
{
    return floor(rational64(position) * rational64(audioRate) / Properties::get().getFrameRate());
}

// static
milliseconds Convert::samplesToTime(int audioRate, int nAudioChannels, samplecount nSamples)
{
    int64_t time =
        static_cast<int64_t>(nSamples) *
        static_cast<int64_t>(sSecond) /
        static_cast<int64_t>(audioRate * nAudioChannels);
    ASSERT_MORE_THAN_EQUALS_ZERO(time);
    return time;
}

// static
double Convert::samplesToSeconds(samplecount nSamples)
{
    return timeToSeconds(samplesToTime(model::Properties::get().getAudioSampleRate(), model::Properties::get().getAudioNumberOfChannels(), nSamples));
}

pts convertFrameRate(pts inputposition, const FrameRate& inputrate, const FrameRate& outputrate)
{
    return floor(rational64(inputposition) / inputrate * outputrate );
}

//static
pts Convert::toProjectFrameRate(pts inputposition, const FrameRate& inputrate)
{
    return convertFrameRate(inputposition, inputrate, Properties::get().getFrameRate());
}

//static
pts Convert::fromProjectFrameRate(pts outputposition, const FrameRate& inputrate)
{
    return convertFrameRate(outputposition, Properties::get().getFrameRate(), inputrate);
}

// static
int Convert::scale(int input, rational64 factor)
{
    return floor(factor * rational64(input));
}

// static
wxSize Convert::scale(const wxSize& input, rational64 factor)
{
    return wxSize(scale(input.x,factor),scale(input.y,factor));
}

// static
wxPoint Convert::scale(const wxPoint& input, rational64 factor)
{
    return wxPoint(scale(input.x,factor),scale(input.y,factor));
}

// static
wxRect Convert::scale(const wxRect& input, rational64 factor)
{
    return wxRect(scale(input.GetPosition(),factor),scale(input.GetSize(),factor));
}

// static
wxSize Convert::sizeInBoundingBox(const wxSize& input, const wxSize& boundingbox, rational64& scaling, bool fill)
{
    rational64 scWidth{ boundingbox.GetWidth(), input.GetWidth() };
    rational64 scHeight{ boundingbox.GetHeight(), input.GetHeight() };

    ASSERT_LESS_THAN_EQUALS(scWidth  * input.GetWidth(), boundingbox.GetWidth());
    ASSERT_LESS_THAN_EQUALS(scHeight * input.GetHeight(), boundingbox.GetHeight());

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
    rational64 dummy;
    return sizeInBoundingBox(input, boundingbox, dummy);
}

// static
double Convert::degreesToRadians(rational64 degrees)
{
    return -1 * boost::rational_cast<double>(degrees) * boost::math::constants::pi<double>() / 180.0;
}

// static
int Convert::doubleToInt(double x)
{
    return (x >= 0.0) ? static_cast<int>(std::floor(x + 0.5)) : static_cast<int>(std::ceil(x - 0.5));     // todo std::trunc?
}

// static
samplecount Convert::audioSamplesToBytes(samplecount nSamples)
{
    return nSamples * AudioChunk::sBytesPerSample;
}

// static 
pts Convert::positionToNewSpeed(pts position, rational64 newSpeed, rational64 oldSpeed)
{
     return boost::rational_cast<pts>(position * oldSpeed / newSpeed);
}

// static 
pts Convert::positionToNormalSpeed(pts position, rational64 speed)
{
     return boost::rational_cast<pts>(speed * position);
}

// static 
int Convert::samplerateToNewSpeed(int samplerate, rational64 newSpeed, rational64 oldSpeed)
{
     return boost::rational_cast<int>(samplerate * oldSpeed / newSpeed);
}

} // namespace