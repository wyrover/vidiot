#include "AudioCodecParameter.h"

#include "Config.h"

namespace model { namespace render {

IMPLEMENTENUM(AudioCodecParameterType);

boost::bimap<AudioCodecParameterType, wxString> AudioCodecParameterTypeConverter::mapToHumanReadibleString = boost::assign::list_of<boost::bimap<AudioCodecParameterType, wxString>::relation >
    (AudioBitRate, _("Bit rate"));

wxString getHumanReadibleName(AudioCodecParameterType id)
{
    return (AudioCodecParameterTypeConverter::mapToHumanReadibleString.left.find(id))->second;
}

void AudioCodecParameterBitrate::set(AVCodecContext* codec)
{
    codec->bit_rate = getValue();
};

}} //namespace