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