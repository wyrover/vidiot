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

#ifndef CONVERT_H
#define CONVERT_H

#include "UtilInt.h"
#include "UtilFrameRate.h"

namespace model {

class Convert
{
public:

    /// \return number of frames to be used for the given time span
    /// \param time time duration in milliseconds
    /// Uses current project's frame rate for the conversion
    static pts timeToPts(milliseconds time);

    /// \return number of frames to be used for the given time span
    /// \param time time duration in milliseconds
    /// Uses current project's frame rate for the conversion
    static pts rationaltimeToPts(rational64 time);

    /// \return number of frames to be used for the given time span
    /// \param time time duration in milliseconds
    /// \param framerate Frame rate (fps) to be used for the conversion
    static pts timeToPts(milliseconds time, const FrameRate& framerate);

    /// \return number of frames to be used for the given time span
    /// \param time time duration in milliseconds
    /// \param framerate Frame rate (fps) to be used for the conversion
    static pts rationaltimeToPts(rational64 time, const FrameRate& framerate);

    /// \return time as a double, in seconds
    /// \param time time duration in milliseconds
    static double timeToSeconds(milliseconds time);

    /// \return time duration in milliseconds
    /// \param position time duration in pts stamps defined by the project's frame rate
    static milliseconds ptsToTime(pts position);

    /// \return time duration in milliseconds
    /// \param pts time duration in pts stamps defined by the given frame rate
    /// \param framerate framerate to be used for converting pts to time
    /// Uses current project's frame rate for the conversion
    static milliseconds ptsToTime(pts position, const FrameRate& framerate);

    /// \return time duration in seconds
    /// \param position time duration in pts stamps defined by the project's frame rate
    static double ptsToSeconds(pts position);

    /// \return time duration in microseconds.
    /// Uses current project's frame rate for the conversion
    static microseconds ptsToMicroseconds(pts position);

    /// \return number of frames to be used for the given time span
    /// \param us time duration in microseconds
    /// Uses current project's frame rate for the conversion
    static pts microsecondsToPts(microseconds us);

    /// If the time is less than an hour (or less then a minute) then no hours (or minutes)
    /// indicators are shown.
    /// \param ms time interval in milliseconds
    /// \param minutesAlways if true, then minutes are shown even if the total time is less than one minute
    /// \param hoursAlways if true, then hours are shown even if the total time is less than one hour
    static wxString msToHumanReadibleString(milliseconds ms, bool minutesAlways = false, bool hoursAlways = false);

    /// Convert a pts value to a human readible string in the form HH:MM:SS.ms
    /// If the time is less than an hour (or less then a minute) then no hours (or minutes)
    /// indicators are shown.
    /// \param minutesAlways if true, then minutes are shown even if the total time is less than one minute
    /// \param hoursAlways if true, then hours are shown even if the total time is less than one hour
    static wxString ptsToHumanReadibleString(pts duration, bool minutesAlways = false, bool hoursAlways = false);

    /// Convert a pts value to the number of audio samples required for each audio channel.
    /// \param audioRate The rate (samples per second )at which audio will be played (44100/48000/etc.)
    /// \param position pts value to be converted
    /// \return number of samples required for this number of pts
    static samplecount ptsToSamplesPerChannel(int audioRate, pts position);

    /// Convert a number of samples to an approximate time value.
    /// \param audioRate The rate (samples per second) at which audio will be played (44100/48000/etc.)
    /// \param nAudioChannels Number of audio channels (speakers)
    /// \param nSamples Number of samples to be converted
    /// \return number of samples required for this number of pts
    static milliseconds samplesToTime(int audioRate, int nAudioChannels, samplecount nSamples);

    /// Convert a number of samples to an approximate time value in seconds, using the project's framerate/number of channels.
    /// \param nSamples Number of samples to be converted
    /// \return number of samples required for this number of pts in seconds
    static double samplesToSeconds(samplecount nSamples);

    /// Determine which timestamp in the project's timebase relates to
    /// a rendered frame timestamp (given an input frame rate)
    /// \param inputposition number of the rendered frame
    /// \param inputrate frame rate of rendering (thus, the rate that is embedded in a file)
    /// \return the related pts value when using the project's time base
    static pts toProjectFrameRate(pts inputposition, const FrameRate& inputrate);

    /// Determine which timestamp in the project's timebase relates to
    /// a rendered frame timestamp (given an input frame rate)
    /// \param outputposition number of the displayed frame (in project's frame rate timebase)
    /// \param inputrate frame rate of rendering (thus, the rate that is embedded in a file)
    /// \return the related to be decoded frame number
    static pts fromProjectFrameRate(pts outputposition, const FrameRate& inputrate);

    static int      scale(int input,               boost::rational<int> factor);
    static wxSize   scale(const wxSize& input,     boost::rational<int> factor);
    static wxPoint  scale(const wxPoint& input,    boost::rational<int> factor);
    static wxRect   scale(const wxRect& input,     boost::rational<int> factor);

    /// Convert an input size to a size fitting entirely in a given bounding
    /// box. Width and height ratio is repected.
    /// \param input input size
    /// \param boundingbox output size is constrained to this bounding box
    /// \param fill if true, then tries to fill the bounding box as much as possible (possibly clipping information). If false, then the entire frame is 'fit' inside the bounding box (possibly with black bands).
    /// \param[out] scaling used scaling by the algorithm
    /// \return maximum size fitting in bounding box with given width/height ratio
    static wxSize sizeInBoundingBox(const wxSize& input, const wxSize& boundingbox, boost::rational<int>& scaling, bool fill = false);
    static wxSize sizeInBoundingBox(const wxSize& input, const wxSize &boundingbox);

    static double degreesToRadians(boost::rational<int> degrees);

    static int doubleToInt(double x);
    static int factorToDigits(boost::rational<int> number, int nDigits);
    static boost::rational<int> digitsToFactor(int number, int nDigits);

    /// Convert a number of audio samples (data for one speaker) to a number of bytes required to store this
    /// \param nSamples Number of audio samples
    /// \return number of required bytes
    static samplecount audioSamplesToBytes(samplecount nSamples);
};
} // namespace

#endif
