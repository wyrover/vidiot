#ifndef CONVERT_H
#define CONVERT_H

#include "UtilInt.h"
#include "UtilFrameRate.h"

namespace model {

class Convert
{
public:

    static pts timeToPts(int time);             ///< @param time time duration in milliseconds
    static int ptsToTime(pts position);         ///< @return time duration in milliseconds
    static int ptsToMicroseconds(pts position); ///< @return time duration in microseconds.
    static pts microsecondsToPts(int us);       ///< @param us time duration in microseconds

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
    static int ptsToSamples(int audioRate, int nAudioChannels, pts position);

    /// Convert a number of samples to an approximate pts value.
    /// \see ptsToSamples
    /// \param audioRate The rate (samples per second) at which audio will be played (44100/48000/etc.)
    /// \param nAudioChannels Number of audio channels (speakers)
    /// \param position pts value to be converted
    /// \return number of samples required for this number of pts
    static pts samplesToPts(int audioRate, int nAudioChannels, int nFrames);

    /// Convert a number of samples (1 sample == data for one speaker) to a number of frames (1 frame == data for all speakers)
    /// \return number of frames stored in given number of samples
    /// \pre nSamples must contain a discrete number of frames (thus nSamples % nChannels == 0)
    static int samplesToFrames(int nChannels, int nSamples);

    /// Convert a number of frames to the required number of samples
    /// \return number of samples stored in given number of frames
    static int framesToSamples(int nChannels, int nFrames);

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

    /// Convert an input size to the maximal size fitting in a given bounding box,
    /// such that one of the axis is equal in size to the corresponding bounding box size.
    /// \param input input size
    /// \param boundingbox output size is constrained to this bounding box in one direction, the other direction is equal or larger than the bounding box
    /// \param[out] scaling used scaling by the algorithm
    /// \return maximum size fitting in bounding box with given width/height ratio
    static wxSize fillBoundingBoxWithMinimalLoss(wxSize input, wxSize boundingbox, boost::rational<int>& scaling);

    static int doubleToInt(double x);
    static int factorToDigits(boost::rational<int> number, int nDigits);
    static boost::rational<int> digitsToFactor(int number, int nDigits);

    /// Convert a number of audio frames (data for all channels) to a number of samples (data for one channel)
    /// \param nFrames Number of audio frames
    /// \param nChannels Number of audio channels (speakers)
    /// \return number of required samples
    static int audioFramesToSamples(int nFrames, int nChannels);

    /// Convert a number of audio frames (data for all channels) to a number of bytes required to store this
    /// \param nFrames Number of audio frames
    /// \param nChannels Number of audio channels (speakers)
    /// \return number of required bytes
    static int audioFramesToBytes(int nFrames, int nChannels);

    /// Convert a number of audio samples (data for one speaker) to a number of bytes required to store this
    /// \param nSamples Number of audio samples
    /// \return number of required bytes
    static int audioSamplesToBytes(int nSamples);
};
} // namespace

#endif // CONVERT_H