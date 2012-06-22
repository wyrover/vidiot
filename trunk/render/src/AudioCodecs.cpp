#include "AudioCodecs.h"

#include <boost/make_shared.hpp>
#include "UtilLog.h"
#include "AudioCodec.h"
#include "AudioCodecParameter.h"

namespace model { namespace render {

// static
AudioCodecMap AudioCodecs::sAudioCodecs;

// static
void AudioCodecs::add(AudioCodec codec)
{
    sAudioCodecs[codec.getId()] = boost::make_shared<AudioCodec>(codec);
}

// static
void AudioCodecs::initialize()
{
    sAudioCodecs.clear();

    add(AudioCodec(CODEC_ID_MP2).
        addParameter(AudioCodecParameterBitrate().enable().setDefault(10000).setMinimum(500).setMaximum(40000)));

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