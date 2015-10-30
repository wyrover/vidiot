// Copyright 2013-2015 Eric Raijmakers.
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
std::map<int,wxString> AudioCodecs::mapToName;

// static
void AudioCodecs::add(const wxString& name, const AudioCodec& codec)
{
    sAudioCodecs[codec.getId()] = boost::make_shared<AudioCodec>(codec);
    mapToName[codec.getId()] = name;
}

// static
void AudioCodecs::initialize()
{
    sAudioCodecs.clear();

    //add(_("No audio"),
    //    AudioCodec(AV_CODEC_ID_NONE));

    add("MP2",
        AudioCodec(AV_CODEC_ID_MP2).
        addParameter(AudioCodecParameterBitrate().setMinimum(500).setMaximum(4000000).setDefault(320000)));

    add("MP3",
        AudioCodec(AV_CODEC_ID_MP3).
        addParameter(AudioCodecParameterBitrate().setMinimum(500).setMaximum(4000000).setDefault(320000)));

    add("AAC",
        AudioCodec(AV_CODEC_ID_AAC).
        addParameter(AudioCodecParameterBitrate().setMinimum(500).setMaximum(4000000).setDefault(320000)));

    add("AMR NB",
        AudioCodec(AV_CODEC_ID_AMR_NB).
        addParameter(AudioCodecParameterBitrate().setMinimum(500).setMaximum(4000000).setDefault(320000)));

    add("FLAC",
        AudioCodec(AV_CODEC_ID_FLAC).
        addParameter(AudioCodecParameterBitrate().setMinimum(500).setMaximum(4000000).setDefault(320000)));

    //add(AudioCodec(AV_CODEC_ID_PCM_S16BE).
    //addParameter(AudioCodecParameterBitrate().setDefault(10000).setMinimum(500).setMaximum(40000)));

    add("PCM 16 bits LE",
        AudioCodec(AV_CODEC_ID_PCM_S16LE).
        addParameter(AudioCodecParameterBitrate().setMinimum(500).setMaximum(4000000).setDefault(320000)));

    add("Vorbis",
        AudioCodec(AV_CODEC_ID_VORBIS).
        addParameter(AudioCodecParameterBitrate().setMinimum(500).setMaximum(4000000).setDefault(320000)));

}

// static
AudioCodecPtr AudioCodecs::getDefault()
{
    return find(AV_CODEC_ID_MP3);
}

// static
std::vector<AVCodecID> AudioCodecs::all()
{
    std::vector<AVCodecID> result;
    for ( AudioCodecMap::value_type entry : sAudioCodecs )
    {
        result.push_back(entry.first);
    }
    return result;
}

// static
AudioCodecPtr AudioCodecs::find(const AVCodecID& id)
{
    AudioCodecMap::const_iterator it = sAudioCodecs.find(id);
    if (it == sAudioCodecs.end())
    {
        return AudioCodecPtr();
    }
    return make_cloned<AudioCodec>(it->second);
}

}} //namespace