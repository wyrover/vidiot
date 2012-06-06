#include "Serialization.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "AudioClip.h"
#include "AudioFile.h"
#include "AudioTrack.h"
#include "AutoFolder.h"
#include "Clip.h"
#include "VideoTransition_CrossFade.h"
#include "EmptyClip.h"
#include "EmptyFile.h"
#include "File.h"
#include "Folder.h"
#include "Sequence.h"
#include "Track.h"
#include "Transition.h"
#include "VideoClip.h"
#include "VideoFile.h"
#include "VideoTrack.h"
#include "VideoTransition.h"

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
    ar.template register_type<Sequence>();
    ar.template register_type<Track>();
    ar.template register_type<Transition>();
    ar.template register_type<VideoClip>();
    ar.template register_type<VideoFile>();
    ar.template register_type<VideoTrack>();
    ar.template register_type<video::transition::CrossFade>();
}

template void registerClasses<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar);
template void registerClasses<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar);

} //namespace