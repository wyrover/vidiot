// Copyright 2013 Eric Raijmakers.
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

#include "AudioChunk.h"
#include "Constants.h"
#include "Properties.h"
#include "UtilFrameRate.h"
#include "UtilLog.h"

namespace model {

// static
pts Convert::timeToPts(int time)
{
    return rationaltimeToPts(rational(time));
}

// static
pts Convert::rationaltimeToPts(boost::rational<int> time)
{
    return rationaltimeToPts(time, Properties::get().getFrameRate());
}

// static
pts Convert::timeToPts(int time, FrameRate framerate)
{
    return rationaltimeToPts(rational(time), framerate);
}

// static
pts Convert::rationaltimeToPts(boost::rational<int> time, FrameRate framerate )
{
    return floor(time / rational(Constants::sSecond) * framerate );
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
int Convert::scale(int input, boost::rational<int> factor)
{
    return floor(factor * boost::rational<int>(input));
}

// static
wxSize Convert::scale(wxSize input, boost::rational<int> factor)
{
    return wxSize(scale(input.x,factor),scale(input.y,factor));
}

// static
wxPoint Convert::scale(wxPoint input, boost::rational<int> factor)
{
    return wxPoint(scale(input.x,factor),scale(input.y,factor));
}

// static
wxRect Convert::scale(wxRect input, boost::rational<int> factor)
{
    return wxRect(scale(input.GetPosition(),factor),scale(input.GetSize(),factor));
}

// static
wxSize Convert::sizeInBoundingBox(wxSize input, wxSize boundingbox, boost::rational<int>& scaling, bool fill)
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
wxSize Convert::sizeInBoundingBox(wxSize input, wxSize boundingbox)
{
    boost::rational<int> dummy;
    return sizeInBoundingBox(input,boundingbox,dummy);
}

// static
int Convert::doubleToInt(double x)
{
    return (x >= 0.0) ? static_cast<int>(std::floor(x + 0.5)) : static_cast<int>(std::ceil(x - 0.5));
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