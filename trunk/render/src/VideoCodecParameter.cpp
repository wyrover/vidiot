#include "VideoCodecParameter.h"

// Include at top, to exclude the intmax macros and use the boost versions
//#undef INTMAX_C
//#undef UINTMAX_C
extern "C" {
#include <avcodec.h>
};

#include <wx/string.h>
#include <boost/assign/list_of.hpp>
#include "Config.h"

namespace model { namespace render {

IMPLEMENTENUM(VideoCodecParameterType);

boost::bimap<VideoCodecParameterType, wxString> VideoCodecParameterTypeConverter::mapToHumanReadibleString = boost::assign::list_of<boost::bimap<VideoCodecParameterType, wxString>::relation >
    (BitRate, _("Bit rate"))
    (GopSize, _("Gop size"))
    (BFrames, _("B Frames"));

wxString getHumandReadibleName(VideoCodecParameterType id)
{
    return (VideoCodecParameterTypeConverter::mapToHumanReadibleString.left.find(id))->second;
}

void VideoCodecParameterBitrate::set(AVCodecContext* codec)
{
    codec->bit_rate = getValue();
};

void VideoCodecParameterBitrateTolerance::set(AVCodecContext* codec)
{
    codec->bit_rate_tolerance = getValue(); // number of bits the bitstream is allowed to diverge from the reference. the reference can be CBR (for CBR pass1) or VBR (for pass2) (unused for constant quantizer encoding.)
};

// todo register types for all of the params.

}} //namespace