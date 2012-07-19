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

    /// Convert a pts value to a number of audio frames. One audio frame is one
    /// element for one speaker. One sample is the data for all involved speakers.
    /// \param audioRate The rate (samples per second )at which audio will be played (44100/48000/etc.)
    /// \param nAudioChannels Number of audio channels (speakers)
    /// \param position pts value to be converted
    /// \return number of frames required for this number of pts
    static int ptsToFrames(int audioRate, int nAudioChannels, pts position);

    /// Convert a number of frames to an approximate pts value.
    /// \see ptsToFrames
    /// \param audioRate The rate (samples per second )at which audio will be played (44100/48000/etc.)
    /// \param nAudioChannels Number of audio channels (speakers)
    /// \param position pts value to be converted
    /// \return number of frames required for this number of pts
    static pts framesToPts(int audioRate, int nAudioChannels, int nFrames);

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

    static int      scale(int input,        double factor);
    static wxSize   scale(wxSize input,     double factor);
    static wxPoint  scale(wxPoint input,    double factor);
    static wxRect   scale(wxRect input,     double factor);

    /// Convert an input size to a size fitting entirely in a given bounding
    /// box. Width and height ratio is repected.
    /// \param input input size
    /// \param boundingbox output size is constrained to this bounding box
    /// \param[out] scaling used scaling by the algorithm
    /// \return maximum size fitting in bounding box with given width/height ratio
    static wxSize sizeInBoundingBox(wxSize input, wxSize boundingbox, double& scaling);
    static wxSize sizeInBoundingBox(wxSize input, wxSize boundingbox);

    /// Convert an input size to the maximal size fitting in a given bounding box,
    /// such that one of the axis is equal in size to the corresponding bounding box size.
    /// \param input input size
    /// \param boundingbox output size is constrained to this bounding box in one direction, the other direction is equal or larger than the bounding box
    /// \param[out] scaling used scaling by the algorithm
    /// \return maximum size fitting in bounding box with given width/height ratio
    static wxSize fillBoundingBoxWithMinimalLoss(wxSize input, wxSize boundingbox, double& scaling);

    static int doubleToInt(double x);
    static int factorToDigits(double number, int nDigits);
    static double digitsToFactor(int number, int nDigits);

    /// Convert a number of audio frames (data for all channels)
    /// to a number of samples (data for one channel)
    /// \param nFrames Number of audio frames
    /// \param nChannels Number of audio channels (speakers)
    /// \return number of required samples
    static int audioFramesToSamples(int nFrames, int nChannels);

    /// Convert a number of audio frames (data for all channels)
    /// to a number of bytes required to hold this datasamples (data for one channel)
    /// \param nFrames Number of audio frames
    /// \param nChannels Number of audio channels (speakers)
    /// \return number of required bytes
    static int audioFramesToBytes(int nFrames, int nChannels);
};
} // namespace

#endif // CONVERT_H