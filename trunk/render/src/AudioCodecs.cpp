#include "AudioCodecs.h"

#include <boost/make_shared.hpp>
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

    add(_("No audio"),
        AudioCodec(CODEC_ID_NONE));

    add(_("MP2"),
        AudioCodec(CODEC_ID_MP2).
        addParameter(AudioCodecParameterBitrate().enable().setMinimum(500).setMaximum(4000000).setDefault(64000)));

    add(_("MP3"),
        AudioCodec(CODEC_ID_MP3).
        addParameter(AudioCodecParameterBitrate().enable().setMinimum(500).setMaximum(4000000).setDefault(64000)));

    add(_("AAC"),
        AudioCodec(CODEC_ID_AAC).
        addParameter(AudioCodecParameterBitrate().enable().setMinimum(500).setMaximum(4000000).setDefault(64000)));

    add(_("AMR NB"),
        AudioCodec(CODEC_ID_AMR_NB).
        addParameter(AudioCodecParameterBitrate().enable().setMinimum(500).setMaximum(4000000).setDefault(64000)));

    add(_("FLAC"),
        AudioCodec(CODEC_ID_FLAC).
        addParameter(AudioCodecParameterBitrate().enable().setMinimum(500).setMaximum(4000000).setDefault(64000)));

    //add(AudioCodec(CODEC_ID_PCM_S16BE).
    //addParameter(AudioCodecParameterBitrate().enable().setDefault(10000).setMinimum(500).setMaximum(40000)));
    // todo check which one of these is supported, little endian or big endian

    add(_("PCM 16 bits Little Endian"),
        AudioCodec(CODEC_ID_PCM_S16LE).
        addParameter(AudioCodecParameterBitrate().enable().setMinimum(500).setMaximum(4000000).setDefault(64000)));

    add(_("Vorbis"),
        AudioCodec(CODEC_ID_VORBIS).
        addParameter(AudioCodecParameterBitrate().enable().setMinimum(500).setMaximum(4000000).setDefault(64000)));
}

// static
AudioCodecPtr AudioCodecs::getDefault()
{
    return find(CODEC_ID_NONE);
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