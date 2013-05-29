#include "Serialization.h"

#include "AudioClip.h"
#include "AudioCodec.h"
#include "AudioCodecParameter.h"
#include "AudioFile.h"
#include "AudioTrack.h"
#include "AudioTransitionFactory.h"
#include "AutoFolder.h"
#include "Clip.h"
#include "EmptyClip.h"
#include "EmptyFile.h"
#include "File.h"
#include "Folder.h"
#include "OutputFormat.h"
#include "Properties.h"
#include "Render.h"
#include "Sequence.h"
#include "Track.h"
#include "Transition.h"
#include "VideoClip.h"
#include "VideoCodec.h"
#include "VideoCodecParameter.h"
#include "VideoFile.h"
#include "VideoTrack.h"
#include "VideoTransition.h"
#include "VideoTransitionFactory.h"

namespace model {

template <class Archive>
void registerClasses(Archive& ar)
{
    ar.template register_type<AudioClip>();
    ar.template register_type<AudioFile>();
    ar.template register_type<AudioTrack>();
    ar.template register_type<AutoFolder>();
//todo?    ar.template register_type<Clip>();
    //todo?ar.template register_type<ClipInterval>();
    ar.template register_type<EmptyClip>();
    ar.template register_type<EmptyFile>();
    ar.template register_type<File>();
    ar.template register_type<Folder>();
    ar.template register_type<Properties>();
    ar.template register_type<render::AudioCodec>();
    ar.template register_type<render::OutputFormat>();
    ar.template register_type<render::Render>();
    ar.template register_type<render::VideoCodec>();
    ar.template register_type<Sequence>();
    ar.template register_type<Track>();
    ar.template register_type<Transition>();
    ar.template register_type<VideoClip>();
    ar.template register_type<VideoFile>();
    ar.template register_type<VideoTrack>();
    model::audio::AudioTransitionFactory::get().registerTypesForSerialization(ar);
    model::video::VideoTransitionFactory::get().registerTypesForSerialization(ar);
    render::registerVideoCodecParameterTypesForSerialization(ar);
    render::registerAudioCodecParameterTypesForSerializaton(ar);
}

template void registerClasses<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar);
template void registerClasses<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar);

} //namespace