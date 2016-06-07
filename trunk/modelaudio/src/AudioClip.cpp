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
}

AudioClip::AudioClip(const AudioClip& other)
    : ClipInterval(other)
{
    VAR_DEBUG(other)(*this);
}

AudioClip* AudioClip::clone() const
{
    return new AudioClip(static_cast<const AudioClip&>(*this));
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
    if (!mPeaks)
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

        KeyFrameMap keyFrames{ getKeyFramesOfPerceivedClip() };
        AudioCompositionParameters fileparameters(parameters);
        fileparameters.setSpeed(getSpeed());
        model::AudioPeaks peaks{ getDataGenerator<AudioFile>()->getPeaks(fileparameters, offset, length) };
        AudioPeaks result;

        if (keyFrames.empty() &&
            (boost::dynamic_pointer_cast<AudioKeyFrame>(getDefaultKeyFrame())->getVolume() == AudioKeyFrame::sVolumeDefault) &&
            (getSpeed() == util::SoundTouch::sDefaultSpeed))
        {
            // Performance optimization for default case (store the peaks for the file only once).
            // Return unchanged peaks from the file.
            return peaks;
        }

        pts position{ 0 };
        int volumeBefore{ boost::dynamic_pointer_cast<AudioKeyFrame>(getFrameAt(position))->getVolume() };
        for (AudioPeak& peak : peaks)
        {
            int volumeAfter{ boost::dynamic_pointer_cast<AudioKeyFrame>(getFrameAt(++position))->getVolume() };
            double volume{ (volumeBefore + volumeAfter) / 200.0 }; // /200: first /2 for the average of the two volumes. Then /100 to get a percentage.
            adjustSampleVolume(volume, peak.first);
            adjustSampleVolume(volume, peak.second);
            volumeBefore = volumeAfter;
        }

        mPeaks.reset(peaks);
    }
    return *mPeaks;
}

//////////////////////////////////////////////////////////////////////////
// KEY FRAMES
//////////////////////////////////////////////////////////////////////////

KeyFramePtr AudioClip::interpolate(KeyFramePtr before, KeyFramePtr after, pts positionBefore, pts position, pts positionAfter) const
{
    return boost::make_shared<AudioKeyFrame>(boost::dynamic_pointer_cast<AudioKeyFrame>(before), boost::dynamic_pointer_cast<AudioKeyFrame>(after), positionBefore, position, positionAfter);
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
