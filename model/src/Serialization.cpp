#include "Serialization.h"

#include "AudioClip.h"
#include "AudioFile.h"
#include "AudioTrack.h"
#include "AutoFolder.h"
#include "Clip.h"
#include "EmptyClip.h"
#include "EmptyFile.h"
#include "File.h"
#include "Folder.h"
#include "Properties.h"
#include "Sequence.h"
#include "Track.h"
#include "Transition.h"
#include "VideoClip.h"
#include "Render.h"
#include "VideoCodec.h"
#include "VideoCodecParameter.h"
#include "AudioCodecParameter.h"
#include "AudioCodec.h"
#include "OutputFormat.h"
#include "VideoFile.h"
#include "VideoTrack.h"
#include "VideoTransition.h"
#include "VideoTransition_CrossFade.h"

namespace model {

template <class Archive>
void registerClasses(Archive& ar)
{
    ar.template register_type<AudioClip>();
    ar.template register_type<AudioFile>();
    ar.template register_type<AudioTrack>();
    ar.template register_type<AutoFolder>();
    ar.template register_type<Clip>();
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
    ar.template register_type<video::transition::CrossFade>();
    ar.template register_type<VideoClip>();
    ar.template register_type<VideoFile>();
    ar.template register_type<VideoTrack>();
    render::registerVideoCodecParameterTypesForSerializaton(ar);
    render::registerAudioCodecParameterTypesForSerializaton(ar);
}

template void registerClasses<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar);
template void registerClasses<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar);

} //namespace