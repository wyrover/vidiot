#ifndef CONVERT_H
#define CONVERT_H

#include "UtilInt.h"

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
    /// @param audioRate The rate (samples per second )at which audio will be played (44100/48000/etc.)
    /// @param nAudioChannels Number of audio channels (speakers)
    /// @param position pts value to be converted
    /// @return number of frames required for this number of pts
    static int ptsToFrames(int audioRate, int nAudioChannels, pts position);

    /// Convert a number of frames to an approximate pts value.
    /// @see ptsToFrames
    /// @param audioRate The rate (samples per second )at which audio will be played (44100/48000/etc.)
    /// @param nAudioChannels Number of audio channels (speakers)
    /// @param position pts value to be converted
    /// @return number of frames required for this number of pts
    static pts framesToPts(int audioRate, int nAudioChannels, int nFrames);
};

} // namespace

#endif // CONVERT_H
