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

#include "AudioClip.h"

#include "AudioCompositionParameters.h"
#include "AudioFile.h"
#include "AudioKeyFrame.h"
#include "AudioPeaks.h"
#include "ClipEvent.h"
#include "Convert.h"
#include "EmptyChunk.h"
#include "Node.h"
#include "Transition.h"

namespace model {

// http://stackoverflow.com/questions/1165026/what-algorithms-could-i-use-for-audio-volume-level
//auto Linear = [](const double& volume, sample& s) { s = std::floor(volume * s); };
//auto IncreasedLowVolumeSensitivity = [](const double& volume, sample& s) { s = std::floor(std::sin(volume * M_PI / 2) * s); }; // Note: Does not work for higher values (sine probably too far, resulting in lower volume instead of higher).

void adjustSampleVolume(const double& volume, sample& s)
{
    double base{ M_E };
    double adjustedSample{ std::trunc((std::pow(base, volume) - 1) / (base - 1) * s) };
    if (adjustedSample < std::numeric_limits<sample>::min())
    {
        adjustedSample = std::numeric_limits<sample>::min();
    }
    else if (adjustedSample > std::numeric_limits<sample>::max())
    {
        adjustedSample = std::numeric_limits<sample>::max();
    }
    s = narrow_cast<sample>(adjustedSample);
};

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

//AudioClip::AudioClip()
//    : ClipInterval()
//{
//    VAR_DEBUG(*this);
//}

AudioClip::AudioClip(const AudioFilePtr& file)
    : ClipInterval(file)
{
    VAR_DEBUG(*this);
    setDefaultKeyFrame(boost::make_shared<AudioKeyFrame>());

    Bind(model::EVENT_CHANGE_CLIP_KEYFRAMES, &AudioClip::onKeyFramesChanged, this);
    Bind(model::EVENT_CHANGE_CLIP_SPEED, &AudioClip::onSpeedChanged, this);
}

// Cached peaks are explicitly copied to avoid cloned clips having duplicates of the peaks.
// Furthermore, during edits (trimming) the cached peaks (for the entire file) can be reused.
AudioClip::AudioClip(const AudioClip& other)
    : ClipInterval(other)
    , mPeaks(other.mPeaks)
{
    VAR_DEBUG(other)(*this);
    Bind(model::EVENT_CHANGE_CLIP_KEYFRAMES, &AudioClip::onKeyFramesChanged, this);
    Bind(model::EVENT_CHANGE_CLIP_SPEED, &AudioClip::onSpeedChanged, this);
}

AudioClip* AudioClip::clone() const
{
    return new AudioClip(static_cast<const AudioClip&>(*this));
}

AudioClip::~AudioClip()
{
    Unbind(model::EVENT_CHANGE_CLIP_SPEED, &AudioClip::onSpeedChanged, this);
    Unbind(model::EVENT_CHANGE_CLIP_KEYFRAMES, &AudioClip::onKeyFramesChanged, this);
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

void AudioClip::clean()
{
    VAR_DEBUG(this);
    mProgress = 0;
    mInputChunk.reset();
    mSoundTouch.reset();
    ClipInterval::clean();
}

//////////////////////////////////////////////////////////////////////////
// ICLIP
//////////////////////////////////////////////////////////////////////////

std::ostream& AudioClip::dump(std::ostream& os) const
{
    os << *this;
    return os;
}

const char* AudioClip::getType() const
{
    return "Audio";
}

//////////////////////////////////////////////////////////////////////////
// IAUDIO
//////////////////////////////////////////////////////////////////////////

AudioChunkPtr AudioClip::getNextAudio(const AudioCompositionParameters& parameters)
{
    if (getNewStartPosition())
    {
        mProgress = *getNewStartPosition(); // Reinitialize mProgress to the last value set in ::moveTo
        mInputChunk.reset(); // Do not use any cached data
        mSoundTouch.reset(); // Do not use any cached data
        invalidateNewStartPosition();
    }

    AudioChunkPtr result;

    pts length{ getLength() };

    if (getSpeed() >= rational64(util::SoundTouch::sMaximumSpeed, util::SoundTouch::sDefaultSpeed) &&
        getSpeed() <= rational64(util::SoundTouch::sMinimumSpeed, util::SoundTouch::sDefaultSpeed) &&
        getSpeed() != rational64(1) &&
        (mSoundTouch == nullptr || mSoundTouch->getSpeed() != getSpeed()))
    {
        mSoundTouch = std::make_unique<util::SoundTouch>(parameters.getSampleRate(), parameters.getNrChannels(), getSpeed());
    }

    if (mProgress < length)
    {

        samplecount requiredSamples{ parameters.getChunkSize() };
        samplecount writtenSamples{ 0 };
        AudioCompositionParameters fileparameters(parameters);

        result = boost::make_shared<AudioChunk>(parameters.getNrChannels(), requiredSamples, true, false);

        if (mSoundTouch)
        {
            while (writtenSamples < requiredSamples)
            {
                if (mSoundTouch->atEnd())
                {
                    break; // No more data available
                }
                else if (mSoundTouch->isEmpty())
                {
                    model::AudioChunkPtr chunk = getDataGenerator<AudioFile>()->getNextAudio(fileparameters);
                    if (chunk->getError())
                    {
                        result->setError();
                    }
                    mSoundTouch->send(chunk);
                }
                else
                {
                    writtenSamples += mSoundTouch->receive(result, writtenSamples, requiredSamples - writtenSamples);
                }
            }
            if (writtenSamples == 0)
            {
                result = nullptr;
                return result;
            }
        }
        else
        {
            fileparameters.setSpeed(getSpeed());
            if (parameters.hasPts())
            {
                pts requiredPts = Convert::positionToNormalSpeed(getOffset() + parameters.getPts(), getSpeed());
                fileparameters.setPts(requiredPts);
            }
            while (writtenSamples < requiredSamples)
            {
                if (mInputChunk &&
                    mInputChunk->getUnreadSampleCount() > 0)
                {
                    if (mInputChunk->getError())
                    {
                        result->setError();
                        break; // No more data available
                    }
                    writtenSamples += mInputChunk->extract(result->getBuffer() + writtenSamples, requiredSamples - writtenSamples);
                }
                else
                {
                    mInputChunk = getDataGenerator<AudioFile>()->getNextAudio(fileparameters);
                    if (!mInputChunk)
                    {
                        break; // No more data available
                    }
                }
            }
        }
        VAR_DEBUG(mProgress)(requiredSamples)(*this);

        if (writtenSamples < requiredSamples)
        {
            // The clip has not provided enough audio data yet (for the pts length of the clip)
            // but there is no more audio data. This can typically happen by using a avi file
            // for which the video data is longer than the audio data. Instead of clipping the
            // extra video part, silence is added here (the user can make the clip shorter if
            // required - thus removing the extra video, but that's a user decision to be made).
            VAR_WARNING(getDescription())(*this)(mProgress)(requiredSamples)(writtenSamples);
            memset(result->getBuffer() + writtenSamples, 0, (requiredSamples - writtenSamples)  * AudioChunk::sBytesPerSample);
        }

        int volumeBefore{ boost::dynamic_pointer_cast<AudioKeyFrame>(getFrameAt(mProgress))->getVolume() };
        int volumeAfter{ boost::dynamic_pointer_cast<AudioKeyFrame>(getFrameAt(mProgress + 1))->getVolume() };

        if (volumeBefore != AudioKeyFrame::sVolumeDefault || volumeAfter != AudioKeyFrame::sVolumeDefault)
        {
            sample* sEnd{ result->getBuffer() + requiredSamples };
            sample* s{ result->getBuffer() };
            int64_t requiredFrames{ narrow_cast<int64_t>(requiredSamples) / parameters.getNrChannels() }; // divide the volume change over this many frames
            int64_t count{ 0 };
            rational64 beginVolume{ volumeBefore,100 };
            rational64 endVolumeDiff{ volumeAfter - volumeBefore, 100 };

            while (s < sEnd)
            {
                double volume{ boost::rational_cast<double>(beginVolume + (endVolumeDiff * rational64{count, requiredFrames})) };
                for (int c{ 0 }; c < parameters.getNrChannels(); ++c, ++s)
                {
                    adjustSampleVolume(volume, *s);
                    // NOT: *s++
                    // Gave problems on Linux because operand s is used twice in the expression,
                    // see http://en.wikipedia.org/wiki/Increment_and_decrement_operators
                }
                count++;
            }
        }
        // else: All samples the same default volume.
    }

    if (result)
    {
        mProgress++;
    }

    return result;
}

//////////////////////////////////////////////////////////////////////////
// AUDIOCLIP
//////////////////////////////////////////////////////////////////////////

AudioPeaks AudioClip::getPeaks(const AudioCompositionParameters& parameters)
{
    pts offset{ getOffset() };
    pts length{ getLength() };
    if (getInTransition())
    {
        boost::optional<pts> left{ getInTransition()->getRight() };
        ASSERT(left);
        ASSERT_NONZERO(*left);
        offset -= *left;
    }
    if (getOutTransition())
    {
        boost::optional<pts> right{ getOutTransition()->getLeft() };
        ASSERT(right);
        ASSERT_NONZERO(*right);
        length += *right;
    }

    if (!mPeaks)
    {
        // Store cached peaks (with adjusted volume/key frames) in save file.
        mPeaks = boost::make_shared<AudioPeaks>();

        // The setPts() & determineChunkSize() below is required for the case where the file has been removed from disk,
        // and the chunk size is used to initialize a chunk of silence.
        maximize(); // When caching, cache the entire file.
        moveTo(0);

        samplecount samplePosition{ 0 };
        samplecount nextRequiredSample{ 0 };
        AudioChunkPtr chunk{ getNextAudio(parameters) };

        size_t data_length{ narrow_cast<size_t>(getLength()) * sPeaksPerPts };

        sample negativePeak = 0;
        sample positivePeak = 0;
        int64_t positiveSum = 0;
        int64_t negativeSum = 0;
        int count = 0;
        double max = std::numeric_limits<sample>::max();

        while (chunk && !chunk->getError())
        {
            samplecount chunksize = chunk->getUnreadSampleCount();
            sample* buffer = chunk->getBuffer();

            for (int i = 0; (i < chunksize) && (mPeaks->size() < data_length); ++i)
            {
                ++count;
                if (*buffer > 0)
                {
                    positiveSum += (*buffer * *buffer);
                    positivePeak = std::max(positivePeak, *buffer);
                }
                else
                {
                    negativeSum += (*buffer * *buffer); // - * - = + !
                    negativePeak = std::min(negativePeak, *buffer);
                }

                if (samplePosition == nextRequiredSample)
                {
                    ASSERT_LESS_THAN_EQUALS_ZERO(negativePeak);
                    ASSERT_MORE_THAN_EQUALS_ZERO(negativeSum);
                    ASSERT_MORE_THAN_EQUALS_ZERO(positivePeak);
                    ASSERT_MORE_THAN_EQUALS_ZERO(positiveSum);
                    mPeaks->emplace_back(AudioPeak(
                    { 
                        { 
                            negativePeak, 
                            positivePeak 
                        },
                        { 
                            -narrow_cast<sample>(std::min(max, std::trunc(sqrt(negativeSum / count)))),  
                            narrow_cast<sample>(std::min(max, std::trunc(sqrt(positiveSum / count)))) 
                        } 
                    }));
                    count = 0;
                    positiveSum = 0;
                    negativeSum = 0;
                    negativePeak = 0;
                    positivePeak = 0;
                    // Note: new speed has been taken into account by getNextAudio already!
                    nextRequiredSample = Convert::ptsToSamplesPerChannel(parameters.getSampleRate(), mPeaks->size()) / sPeaksPerPts;
                }
                ++samplePosition;
                ++buffer;
            }
            chunk = getNextAudio(parameters);
        }

        KeyFrameMap keyFrames{ getKeyFramesOfPerceivedClip() };
        if (keyFrames.empty() &&
            (boost::dynamic_pointer_cast<AudioKeyFrame>(getDefaultKeyFrame())->getVolume() == AudioKeyFrame::sVolumeDefault) &&
            (getSpeed() == util::SoundTouch::sDefaultSpeed))
        {
            // Performance optimization for default case.
        }
        else
        {
            pts position{ 0 };
            int volumeBefore{ boost::dynamic_pointer_cast<AudioKeyFrame>(getFrameAt(position / sPeaksPerPts))->getVolume() };
            for (AudioPeak& peak : *mPeaks)
            {
                ++position;
                int volumeAfter{ boost::dynamic_pointer_cast<AudioKeyFrame>(getFrameAt(position / sPeaksPerPts))->getVolume() };
                double volume{ (volumeBefore + volumeAfter) / 200.0 }; // /200: first /2 for the average of the two volumes. Then /100 to get a percentage.
                adjustSampleVolume(volume, peak.first.first);
                adjustSampleVolume(volume, peak.first.second);     // todo obsolete?
                adjustSampleVolume(volume, peak.second.first);
                adjustSampleVolume(volume, peak.second.second);
                volumeBefore = volumeAfter;
            }
        }

        // No asserts on 'enough data' here:
        // The audio clip may be slightly larger than the audio file data. This can be caused by the clip having (typically) the same length as a linked video clip.
        // The video data in a file may be slightly longer than the audio data, resulting in such a difference. Instead of truncating the video, the audio is extended
        // with silence, leaving the truncating (the choice) to the user.

    }


    int offsetInPeaks = offset * sPeaksPerPts;
    int lengthInPeaks = length * sPeaksPerPts;
    int nPeaks = mPeaks->size();

    auto itBegin = mPeaks->cbegin() + std::min(offsetInPeaks, nPeaks);
    auto itEnd = mPeaks->cbegin() + std::min(offsetInPeaks + lengthInPeaks, nPeaks);
    if ((std::distance(mPeaks->cbegin(), itBegin) < nPeaks) &&
        (std::distance(mPeaks->cbegin(), itEnd) <= nPeaks))
    {
        return std::move(AudioPeaks(itBegin, itEnd));
    }
    if (static_cast<int>(mPeaks->size()) < length)
    {
        // Ensure resulting peaks length equals length of clip. Add 'silence' if required.
        mPeaks->resize(length, AudioPeak({ { 0,0 },{ 0,0 } }));
    }

    return AudioPeaks();
}

//////////////////////////////////////////////////////////////////////////
// KEY FRAMES
//////////////////////////////////////////////////////////////////////////

KeyFramePtr AudioClip::interpolate(KeyFramePtr before, KeyFramePtr after, pts positionBefore, pts position, pts positionAfter) const
{
    return boost::make_shared<AudioKeyFrame>(boost::dynamic_pointer_cast<AudioKeyFrame>(before), boost::dynamic_pointer_cast<AudioKeyFrame>(after), positionBefore, position, positionAfter);
}

//////////////////////////////////////////////////////////////////////////
// CLIP INTERVAL EVENTS
//////////////////////////////////////////////////////////////////////////

void AudioClip::onKeyFramesChanged(EventChangeClipKeyFrames& event)
{
    mPeaks = nullptr;
    event.Skip();
}

void AudioClip::onSpeedChanged(EventChangeClipSpeed& event)
{
    mPeaks = nullptr;
    event.Skip();
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const AudioClip& obj)
{
    os << static_cast<const ClipInterval&>(obj) << '|'
        << std::setw(8) << obj.mProgress << '|';
    obj.logKeyFramesAs<AudioKeyFrame>(os);
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void AudioClip::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ClipInterval);
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(IAudio);
        if (version < 3)
        {
            int mVolume{ 100 };
            if (version > 1)
            {
                ar & BOOST_SERIALIZATION_NVP(mVolume);
            }
            AudioKeyFramePtr keyFrame{ boost::make_shared<AudioKeyFrame>() };
            keyFrame->setVolume(mVolume);
            setDefaultKeyFrame(keyFrame);
        }
        if (version >= 4)
        {
            ar & BOOST_SERIALIZATION_NVP(mPeaks);
        }
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void AudioClip::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void AudioClip::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::AudioClip)
