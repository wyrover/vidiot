#ifndef AUDIO_FRAME_COMPOSITION_PARAMETERS_H
#define AUDIO_FRAME_COMPOSITION_PARAMETERS_H

#include "IAudio.h"
#include "UtilInt.h"

namespace model {

class AudioCompositionParameters
{
public:

    //////////////////////////////////////////////////////////////////////////
    // INITIALIZATION
    //////////////////////////////////////////////////////////////////////////

    AudioCompositionParameters();
    AudioCompositionParameters(const AudioCompositionParameters& other);

    //////////////////////////////////////////////////////////////////////////
    // GET/SET
    //////////////////////////////////////////////////////////////////////////

    AudioCompositionParameters& setSampleRate(int audiorate);
    int getSampleRate() const;

    AudioCompositionParameters& setNrChannels(int nChannels);
    int getNrChannels() const;

    samplecount getChunkSize() const;

    //////////////////////////////////////////////////////////////////////////
    // CONVERSION HELPERS
    //////////////////////////////////////////////////////////////////////////

    /// Convert a pts value to a number of audio samples, given this set of parameters.
    /// \param position pts value to be converted
    int ptsToSamples(pts position) const;

    /// Convert a number of samples to an approximate pts value, given this set of parameters.
    /// \return number of samples required for this number of pts
    pts samplesToPts(int nFrames) const;

    /// Convert a number of samples (1 sample == data for one speaker) to a number of frames (1 frame == data for all speakers)
    /// \return number of frames stored in given number of samples
    /// \pre nSamples must contain a discrete number of frames (thus nSamples % nChannels == 0)
    int samplesToFrames(int nSamples) const;

    /// Convert a number of frames to the required number of samples
    /// \return number of samples stored in given number of frames
    int framesToSamples(int nFrames) const;

private:

    //////////////////////////////////////////////////////////////////////////
    // MEMBERS
    //////////////////////////////////////////////////////////////////////////

    int mSampleRate;    ///< The rate (samples per second) at which audio will be played (44100/48000/etc.)
    int mNrChannels;    ///< Number of audio channels (speakers)

    //////////////////////////////////////////////////////////////////////////
    // LOGGING
    //////////////////////////////////////////////////////////////////////////

    friend std::ostream& operator<<( std::ostream& os, const AudioCompositionParameters& obj );

};

} // namespace

#endif // VIDEO_FRAME_COMPOSITION_PARAMETERS_H