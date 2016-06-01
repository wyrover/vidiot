// Copyright 2016 Eric Raijmakers.
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

#include "UtilSoundTouch.h"

#include <SoundTouch.h>
#include "AudioChunk.h"

namespace util {

const int SoundTouch::sMinimumSpeed = 50;
const int SoundTouch::sMaximumSpeed = 200;
const int SoundTouch::sDefaultSpeed = 100;

SoundTouch::SoundTouch(int samplerate, int channels, rational64 speed)
    : mSoundTouch{ std::make_unique<soundtouch::SoundTouch>() }
    , mSampleRate{ samplerate }
    , mChannels{ channels }
    , mSpeed{ speed }
{
    mSoundTouch->setSampleRate(mSampleRate);
    mSoundTouch->setChannels(mChannels);
    mSoundTouch->setTempo(1.0);
    int newspeed{ narrow_cast<int>(trunc(boost::rational_cast<double>(mSpeed * rational64(100, 1)))) };
    mSoundTouch->setTempoChange(newspeed - sDefaultSpeed);
    mSoundTouch->setRate(1.0);
    mSoundTouch->setRateChange(0);
    mSoundTouch->setSetting(SETTING_USE_AA_FILTER, 0);//1
    mSoundTouch->setSetting(SETTING_SEQUENCE_MS, 40);    // Optimize for speech
    mSoundTouch->setSetting(SETTING_SEEKWINDOW_MS, 15);  // Optimize for speech
    mSoundTouch->setSetting(SETTING_OVERLAP_MS, 8);      // Optimize for speech
}

SoundTouch::~SoundTouch()
{
    mSoundTouch->clear();
}



void SoundTouch::send(model::AudioChunkPtr chunk)
{
    if (chunk == nullptr)
    {
        mAtEnd = true;
        return;
    }
#ifdef SOUNDTOUCH_INTEGER_SAMPLES
    mSoundTouch->putSamples(reinterpret_cast<const short *>(chunk->getUnreadSamples()), chunk->getUnreadSampleCount() / mChannels);
#else
    float *convertTo = new float[chunk->getUnreadSampleCount()];
    sample* s = chunk->getUnreadSamples();
    float* t = convertTo;
    static const float M = 32767.0; // 2 ^ 15
    for (int i = 0; i < chunk->getUnreadSampleCount(); ++i)
    {
        *t++ = static_cast<float>(*s++) / M;
    }
    mSoundTouch->putSamples(convertTo, chunk->getUnreadSampleCount() / mChannels);
    delete[] convertTo;
#endif
}

samplecount SoundTouch::receive(model::AudioChunkPtr chunk, samplecount offset, samplecount nSamplesRequired)
{
    ASSERT(!atEnd());
    int nFramesAvailable = mSoundTouch->numSamples();
    int nFramesRequired = nSamplesRequired / mChannels;
#ifdef SOUNDTOUCH_INTEGER_SAMPLES
    samplecount nFrames = mSoundTouch->receiveSamples(reinterpret_cast<short*>(chunk->getBuffer()) + offset, nFramesRequired);
    ASSERT_LESS_THAN_EQUALS(nFrames, nFramesAvailable);
#else // SOUNDTOUCH_FLOAT_SAMPLES
    static const float M = 32767.0; // 2 ^ 15
    float* convertFrom = new float[nSamplesRequired];
    samplecount nFrames = mSoundTouch->receiveSamples(convertFrom, nFramesRequired);
    ASSERT_LESS_THAN_EQUALS(nFrames, nFramesAvailable);
    ASSERT_LESS_THAN_EQUALS(nFrames, nFramesRequired);

    int nSamples = nFrames * mChannels;
    sample* t = chunk->getBuffer() + offset;
    float* s = convertFrom;
    for (int i = 0; i < nSamples; ++i)
    {
        float f = *s++ * M;
        if (f < -M) { f = -M; }
        if (f > M - 1) { f = M - 1; }
        *t++ = static_cast<sample>(f);
    }
    delete[] convertFrom;
#endif
    if (mAtEnd)
    {
        // Fill remainder with silence
        // todo removeafter testing on both platforms.
        // check to see if memset with 0 size is allowed.
        memset(chunk->getBuffer() + offset + nFramesAvailable, 0, 0);
        memset(chunk->getBuffer() + offset + nFramesAvailable, 0, (nFramesRequired - nFramesAvailable) * model::AudioChunk::sBytesPerSample);
        nFrames = nFramesRequired;
    }
    return nFrames * mChannels;
}

bool SoundTouch::isEmpty()
{
    return mSoundTouch->isEmpty() != 0;
}

bool SoundTouch::atEnd()
{
    return mAtEnd && isEmpty();
}

rational64 SoundTouch::getSpeed() const
{
    return mSpeed;
}

}