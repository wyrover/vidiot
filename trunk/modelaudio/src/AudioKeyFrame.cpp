// Copyright 2015-2016 Eric Raijmakers.
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

#include "AudioKeyFrame.h"

#include "Config.h"
#include "Convert.h"
#include "EmptyFrame.h"
#include "Properties.h"
#include "StatusBar.h"
#include "UtilSerializeBoost.h"
#include "UtilSerializeWxwidgets.h"
#include "AudioCompositionParameters.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

constexpr int AudioKeyFrame::sVolumeMin;
constexpr int AudioKeyFrame::sVolumeMax;
constexpr int AudioKeyFrame::sVolumeDefault;

AudioKeyFrame::AudioKeyFrame()
    : KeyFrame{ false }
    , mVolume(sVolumeDefault)
{
    VAR_DEBUG(*this);
}

AudioKeyFrame::AudioKeyFrame(AudioKeyFramePtr before, AudioKeyFramePtr after, pts positionBefore, pts position, pts positionAfter)
    : KeyFrame{ false }
    , mVolume(sVolumeDefault)
{
    ASSERT_LESS_THAN(positionBefore, position);
    ASSERT_LESS_THAN(position, positionAfter);
    rational64 factor{ position - positionBefore, positionAfter - positionBefore };
    ASSERT_MORE_THAN_EQUALS_ZERO(factor);
    ASSERT_LESS_THAN(factor, 1);

    mVolume = before->getVolume() + boost::rational_cast<int>(factor * (rational64(after->getVolume() - before->getVolume())));
}

AudioKeyFrame::AudioKeyFrame(const AudioKeyFrame& other)
    : KeyFrame{ other }
    , mVolume{ other.mVolume }
{
    VAR_DEBUG(other)(*this);
}

AudioKeyFrame* AudioKeyFrame::clone() const
{
    return new AudioKeyFrame(static_cast<const AudioKeyFrame&>(*this));
}

AudioKeyFrame::~AudioKeyFrame()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// GET/SET
//////////////////////////////////////////////////////////////////////////

int AudioKeyFrame::getVolume() const
{
    return mVolume;
}

void AudioKeyFrame::setVolume(int volume)
{
    ASSERT(!isInterpolated())(*this);
    ASSERT_MORE_THAN_EQUALS(volume, sVolumeMin);
    ASSERT_LESS_THAN_EQUALS(volume, sVolumeMax);
    mVolume = volume;
}

//////////////////////////////////////////////////////////////////////////
// LOGGING
//////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& os, const AudioKeyFrame& obj)
{
    os << static_cast<const KeyFrame&>(obj) << '|'
        << std::setw(4) << obj.mVolume;
    return os;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void AudioKeyFrame::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(KeyFrame);
        ar & BOOST_SERIALIZATION_NVP(mVolume);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void AudioKeyFrame::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void AudioKeyFrame::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::AudioKeyFrame)
