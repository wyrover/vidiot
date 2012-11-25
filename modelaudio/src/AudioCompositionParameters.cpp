#include "AudioCompositionParameters.h"

#include "Config.h"
#include "Convert.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AudioCompositionParameters::AudioCompositionParameters()
    :   mSampleRate(Config::ReadLong(Config::sPathDefaultAudioSampleRate))
    ,   mNrChannels(Config::ReadLong(Config::sPathDefaultAudioChannels))
{
}

AudioCompositionParameters::AudioCompositionParameters(const AudioCompositionParameters& other)
    :   mSampleRate(other.mSampleRate)
    ,   mNrChannels(other.mNrChannels)
{
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

AudioCompositionParameters& AudioCompositionParameters::setSampleRate(int audiorate)
{
    mSampleRate = audiorate;
    return *this;
}

int AudioCompositionParameters::getSampleRate() const
{
    return mSampleRate;
}

AudioCompositionParameters& AudioCompositionParameters::setNrChannels(int nChannels)
{
    mNrChannels = nChannels;
    return *this;
}

int AudioCompositionParameters::getNrChannels() const
{
    return mNrChannels;
}

//////////////////////////////////////////////////////////////////////////
// CONVERSION HELPERS
//////////////////////////////////////////////////////////////////////////

int AudioCompositionParameters::ptsToSamples(pts position) const
{
    return Convert::ptsToSamples(position, mSampleRate, mNrChannels);
}

pts AudioCompositionParameters::samplesToPts(int nFrames) const
{
    return Convert::samplesToPts(nFrames, mSampleRate, mNrChannels);
}

int AudioCompositionParameters::samplesToFrames(int nSamples) const
{
    return Convert::samplesToFrames(mNrChannels, nSamples);
}

int AudioCompositionParameters::framesToSamples(int nFrames) const
{
    return Convert::framesToSamples(mNrChannels, nFrames);
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<( std::ostream& os, const AudioCompositionParameters& obj )
{
    os << &obj << '|' << obj.mSampleRate << '|' << obj.mNrChannels;
    return os;
}

} //namespace