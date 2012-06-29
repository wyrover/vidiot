#include "AudioCodecParameter.h"

extern "C" {
#include <avcodec.h>
};

#include <wx/string.h>
#include <boost/assign/list_of.hpp>
#include "Config.h"

namespace model { namespace render {

IMPLEMENTENUM(AudioCodecParameterType);

boost::bimap<AudioCodecParameterType, wxString> AudioCodecParameterTypeConverter::mapToHumanReadibleString = boost::assign::list_of<boost::bimap<AudioCodecParameterType, wxString>::relation >
    (BitRate, _("Bit rate"));

wxString getHumanReadibleName(AudioCodecParameterType id)
{
    return (AudioCodecParameterTypeConverter::mapToHumanReadibleString.left.find(id))->second;
}

void AudioCodecParameterBitrate::set(AVCodecContext* codec)
{
    codec->bit_rate = getValue();
};

// todo register types for all of the params.

}} //namespace