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

#include "AudioCodecParameter.h"

#include "Config.h"

namespace model { namespace render {

IMPLEMENTENUM(AudioCodecParameterType);

boost::bimap<AudioCodecParameterType, wxString> AudioCodecParameterTypeConverter::mapToHumanReadibleString = boost::assign::list_of<boost::bimap<AudioCodecParameterType, wxString>::relation >
    (AudioBitRate, _("Bit rate"));

wxString getHumanReadibleName(const AudioCodecParameterType& id)
{
    ASSERT(AudioCodecParameterTypeConverter::mapToHumanReadibleString.left.find(id) != AudioCodecParameterTypeConverter::mapToHumanReadibleString.left.end())(id);
    return (AudioCodecParameterTypeConverter::mapToHumanReadibleString.left.find(id))->second;
}

AudioCodecParameterType getAudioCodecIdFromHumanReadibleName(const wxString& name)
{
    ASSERT(AudioCodecParameterTypeConverter::mapToHumanReadibleString.right.find(name) != AudioCodecParameterTypeConverter::mapToHumanReadibleString.right.end())(name);
    return (AudioCodecParameterTypeConverter::mapToHumanReadibleString.right.find(name))->second;
}

void AudioCodecParameterBitrate::set(AVCodecContext* codec)
{
    codec->bit_rate = getValue();
};

}} //namespace

BOOST_CLASS_EXPORT_IMPLEMENT(model::render::AudioCodecParameterBitrate)