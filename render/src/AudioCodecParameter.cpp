#include "AudioCodecParameter.h"

#include <wx/string.h>
#include <boost/assign/list_of.hpp>
#include "Config.h"

namespace model { namespace render {

IMPLEMENTENUM(AudioCodecParameterType);

boost::bimap<AudioCodecParameterType, wxString> AudioCodecParameterTypeConverter::mapToHumanReadibleString = boost::assign::list_of<boost::bimap<AudioCodecParameterType, wxString>::relation >
    (BitRate, _("Bit rate"));

wxString getHumandReadibleName(AudioCodecParameterType id)
{
    return (AudioCodecParameterTypeConverter::mapToHumanReadibleString.left.find(id))->second;
}

// todo register types for all of the params.

}} //namespace