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
    static pts timeToPts(int time);

    /// \return number of frames to be used for the given time span
    /// \param time time duration in milliseconds
    /// Uses current project's frame rate for the conversion
    static pts rationaltimeToPts(boost::rational<int> time);

    /// \return number of frames to be used for the given time span
    /// \param time time duration in milliseconds
    /// \param framerate Frame rate (fps) to be used for the conversion
    static pts timeToPts(int time, FrameRate framerate);

    /// \return number of frames to be used for the given time span
    /// \param time time duration in milliseconds
    /// \param framerate Frame rate (fps) to be used for the conversion
    static pts rationaltimeToPts(boost::rational<int> time, FrameRate framerate);

    /// \return time duration in milliseconds
    /// Uses current project's frame rate for the conversion
    static int ptsToTime(pts position);

    /// \return time duration in microseconds.
    /// Uses current project's frame rate for the conversion
    static int ptsToMicroseconds(pts position);

    /// \return number of frames to be used for the given time span
    /// \param us time duration in microseconds
    /// Uses current project's frame rate for the conversion
    static pts microsecondsToPts(int us);

    /// Convert a time in ms to a human readible string in the form HH:MM:SS.ms
    /// If the time is less than an hour (or less then a minute) then no hours (or minutes)
    /// indicators are shown.
    /// \param ms time interval in milliseconds
    static wxString msToHumanReadibleString(int ms);

    /// Convert a pts value to a human readible string in the form HH:MM:SS.ms
    /// If the time is less than an hour (or less then a minute) then no hours (or minutes)
    /// indicators are shown.
    static wxString ptsToHumanReadibleString(pts duration);

    /// Convert a pts value to a number of audio samples. One audio sample is one
    /// element for one speaker.
    /// \param audioRate The rate (samples per second )at which audio will be played (44100/48000/etc.)
    /// \param nAudioChannels Number of audio channels (speakers)
    /// \param position pts value to be converted
    /// \return number of samples required for this number of pts
    static samplecount ptsToSamples(int audioRate, int nAudioChannels, pts position);

    /// Convert a number of samples to an approximate pts value.
    /// \see ptsToSamples
    /// \param audioRate The rate (samples per second) at which audio will be played (44100/48000/etc.)
    /// \param nAudioChannels Number of audio channels (speakers)
    /// \param nSamples Number of samples to be converted
    /// \return number of samples required for this number of pts
    static pts samplesToPts(int audioRate, int nAudioChannels, samplecount nSamples);

    /// Convert a number of samples (1 sample == data for one speaker) to a number of frames (1 frame == data for all speakers)
    /// \return number of frames stored in given number of samples
    /// \pre nSamples must contain a discrete number of frames (thus nSamples % nChannels == 0)
    static samplecount samplesToFrames(int nChannels, samplecount nSamples);

    /// Convert a number of frames to the required number of samples
    /// \return number of samples stored in given number of frames
    static samplecount framesToSamples(int nChannels, samplecount nFrames);

    /// Determine which timestamp in the project's timebase relates to
    /// a rendered frame timestamp (given an input frame rate)
    /// \param inputposition number of the rendered frame
    /// \param inputrate frame rate of rendering (thus, the rate that is embedded in a file)
    /// \return the related pts value when using the project's time base
    static pts toProjectFrameRate(pts inputposition, FrameRate inputrate);

    /// Determine which timestamp in the project's timebase relates to
    /// a rendered frame timestamp (given an input frame rate)
    /// \param outputposition number of the displayed frame (in project's frame rate timebase)
    /// \param inputrate frame rate of rendering (thus, the rate that is embedded in a file)
    /// \return the related to be decoded frame number
    static pts fromProjectFrameRate(pts outputposition, FrameRate inputrate);

    static int      scale(int input,        boost::rational<int> factor);
    static wxSize   scale(wxSize input,     boost::rational<int> factor);
    static wxPoint  scale(wxPoint input,    boost::rational<int> factor);
    static wxRect   scale(wxRect input,     boost::rational<int> factor);

    /// Convert an input size to a size fitting entirely in a given bounding
    /// box. Width and height ratio is repected.
    /// \param input input size
    /// \param boundingbox output size is constrained to this bounding box
    /// \param fill if true, then tries to fill the bounding box as much as possible (possibly clipping information). If false, then the entire frame is 'fit' inside the bounding box (possibly with black bands).
    /// \param[out] scaling used scaling by the algorithm
    /// \return maximum size fitting in bounding box with given width/height ratio
    static wxSize sizeInBoundingBox(wxSize input, wxSize boundingbox, boost::rational<int>& scaling, bool fill = false);
    static wxSize sizeInBoundingBox(wxSize input, wxSize boundingbox);

    static int doubleToInt(double x);
    static int factorToDigits(boost::rational<int> number, int nDigits);
    static boost::rational<int> digitsToFactor(int number, int nDigits);

    /// Convert a number of audio frames (data for all channels) to a number of samples (data for one channel)
    /// \param nFrames Number of audio frames
    /// \param nChannels Number of audio channels (speakers)
    /// \return number of required samples
    static samplecount audioFramesToSamples(samplecount nFrames, int nChannels);

    /// Convert a number of audio frames (data for all channels) to a number of bytes required to store this
    /// \param nFrames Number of audio frames
    /// \param nChannels Number of audio channels (speakers)
    /// \return number of required bytes
    static samplecount audioFramesToBytes(samplecount nFrames, int nChannels);

    /// Convert a number of audio samples (data for one speaker) to a number of bytes required to store this
    /// \param nSamples Number of audio samples
    /// \return number of required bytes
    static samplecount audioSamplesToBytes(samplecount nSamples);
};
} // namespace

#endif // CONVERT_H