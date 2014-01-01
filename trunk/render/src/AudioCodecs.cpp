// Copyright 2013,2014 Eric Raijmakers.
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

#include "AudioCodecs.h"

#include "UtilLog.h"
#include "AudioCodec.h"
#include "AudioCodecParameter.h"

namespace model { namespace render {

// static
AudioCodecMap AudioCodecs::sAudioCodecs;

// static
boost::bimap<int,wxString> AudioCodecs::mapToName;

// static
void AudioCodecs::add(wxString name, AudioCodec codec)
{
    sAudioCodecs[codec.getId()] = boost::make_shared<AudioCodec>(codec);
    typedef boost::bimap<int, wxString> bimap;
    mapToName.insert( bimap::value_type(codec.getId(), name) );
}

// static
void AudioCodecs::initialize()
{
    sAudioCodecs.clear();

    //add(_("No audio"),
    //    AudioCodec(CODEC_ID_NONE));

    add(_("MP2"),
        AudioCodec(CODEC_ID_MP2).
        addParameter(AudioCodecParameterBitrate().setMinimum(500).setMaximum(4000000).setDefault(320000)));

    add(_("MP3"),
        AudioCodec(CODEC_ID_MP3).
        addParameter(AudioCodecParameterBitrate().setMinimum(500).setMaximum(4000000).setDefault(320000)));

    add(_("AAC"),
        AudioCodec(CODEC_ID_AAC).
        addParameter(AudioCodecParameterBitrate().setMinimum(500).setMaximum(4000000).setDefault(320000)));

    add(_("AMR NB"),
        AudioCodec(CODEC_ID_AMR_NB).
        addParameter(AudioCodecParameterBitrate().setMinimum(500).setMaximum(4000000).setDefault(320000)));

    add(_("FLAC"),
        AudioCodec(CODEC_ID_FLAC).
        addParameter(AudioCodecParameterBitrate().setMinimum(500).setMaximum(4000000).setDefault(320000)));

    //add(AudioCodec(CODEC_ID_PCM_S16BE).
    //addParameter(AudioCodecParameterBitrate().setDefault(10000).setMinimum(500).setMaximum(40000)));

    add(_("PCM 16 bits Little Endian"),
        AudioCodec(CODEC_ID_PCM_S16LE).
        addParameter(AudioCodecParameterBitrate().setMinimum(500).setMaximum(4000000).setDefault(320000)));

    add(_("Vorbis"),
        AudioCodec(CODEC_ID_VORBIS).
        addParameter(AudioCodecParameterBitrate().setMinimum(500).setMaximum(4000000).setDefault(320000)));
}

// static
AudioCodecPtr AudioCodecs::getDefault()
{
    return find(CODEC_ID_MP3);
}

// static
AudioCodecPtr AudioCodecs::find(CodecID id)
{
    AudioCodecMap::const_iterator it = sAudioCodecs.find(id);
    if (it == sAudioCodecs.end())
    {
        return AudioCodecPtr();
    }
    return make_cloned<AudioCodec>(it->second);
}

}} //namespace