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

#include "AudioTrack.h"

#include "Config.h"
#include "AudioClip.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AudioTrack::AudioTrack()
:	Track()
{
    VAR_DEBUG(this);
    setHeight(Config::get().read<int>(Config::sPathTimelineDefaultAudioTrackHeight));
}

AudioTrack::AudioTrack(const AudioTrack& other)
:	Track(other)
{
    VAR_DEBUG(this);
}

AudioTrack* AudioTrack::clone() const
{
    return new AudioTrack(static_cast<const AudioTrack&>(*this));
}

AudioTrack::~AudioTrack()
{
    VAR_DEBUG(this);
}

//////////////////////////////////////////////////////////////////////////
// ICONTROL
//////////////////////////////////////////////////////////////////////////

void AudioTrack::clean()
{
    VAR_DEBUG(this);
    Track::clean();
}

//////////////////////////////////////////////////////////////////////////
// PLAYBACK
//////////////////////////////////////////////////////////////////////////

AudioChunkPtr AudioTrack::getNextAudio(const AudioCompositionParameters& parameters)
{
    AudioChunkPtr audioChunk;

    while (!audioChunk && !iterate_atEnd())
    {
        model::IAudioPtr audio = boost::dynamic_pointer_cast<IAudio>(iterate_get());
        model::IClipPtr clip = boost::dynamic_pointer_cast<IClip>(iterate_get());
        audioChunk = audio->getNextAudio(AudioCompositionParameters(parameters).adjustPts(-clip->getLeftPts()));
        if (!audioChunk)
        {
            iterate_next();
            if (!iterate_atEnd())
            {
                iterate_get()->moveTo(0);
            }
        }
    }

    return audioChunk;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void AudioTrack::serialize(Archive & ar, const unsigned int version)
{
    try
    {
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Track);
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(IAudio);
    }
    catch (boost::exception &e)                  { VAR_ERROR(boost::diagnostic_information(e)); throw; }
    catch (std::exception& e)                    { VAR_ERROR(e.what());                         throw; }
    catch (...)                                  { LOG_ERROR;                                   throw; }
}
template void AudioTrack::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive& ar, const unsigned int archiveVersion);
template void AudioTrack::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive& ar, const unsigned int archiveVersion);

} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::AudioTrack)