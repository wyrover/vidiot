// Copyright 2013 Eric Raijmakers.
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

#include "UtilLog.h"
#include "AudioClip.h"

namespace model {

//////////////////////////////////////////////////////////////////////////
// INITIALIZATION
//////////////////////////////////////////////////////////////////////////

AudioTrack::AudioTrack()
:	Track()
{
    VAR_DEBUG(this);
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
        audioChunk = boost::dynamic_pointer_cast<IAudio>(iterate_get())->getNextAudio(parameters);
        if (!audioChunk)
        {
            iterate_next();
            if (!iterate_atEnd())
            {
                iterate_get()->moveTo(0);
            }
        }
    }

    VAR_AUDIO(audioChunk);
    return audioChunk;
}

//////////////////////////////////////////////////////////////////////////
// SERIALIZATION
//////////////////////////////////////////////////////////////////////////

template<class Archive>
void AudioTrack::serialize(Archive & ar, const unsigned int version)
{
    ar & boost::serialization::base_object<Track>(*this);
    ar & boost::serialization::base_object<IAudio>(*this);
}
template void AudioTrack::serialize<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar, const unsigned int archiveVersion);
template void AudioTrack::serialize<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar, const unsigned int archiveVersion);

} //namespace