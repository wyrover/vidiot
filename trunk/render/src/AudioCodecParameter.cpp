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

#include "AudioCodecParameter.h"

#include "Config.h"

namespace model { namespace render {

IMPLEMENTENUM(AudioCodecParameterType);

boost::bimap<AudioCodecParameterType, wxString> AudioCodecParameterTypeConverter::mapToHumanReadibleString = boost::assign::list_of<boost::bimap<AudioCodecParameterType, wxString>::relation >
    (AudioBitRate, _("Bit rate"));

template < class Archive >
void registerAudioCodecParameterTypesForSerializaton(Archive& ar)
{
    ar.template register_type<render::AudioCodecParameterBitrate>();
}

template void registerAudioCodecParameterTypesForSerializaton<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar);
template void registerAudioCodecParameterTypesForSerializaton<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar);

wxString getHumanReadibleName(AudioCodecParameterType id)
{
    ASSERT(AudioCodecParameterTypeConverter::mapToHumanReadibleString.left.find(id) != AudioCodecParameterTypeConverter::mapToHumanReadibleString.left.end())(id);
    return (AudioCodecParameterTypeConverter::mapToHumanReadibleString.left.find(id))->second;
}

void AudioCodecParameterBitrate::set(AVCodecContext* codec)
{
    codec->bit_rate = getValue();
};

}} //namespace