#include "AudioTransitionFactory.h"

#include "AudioTransition.h"
#include "AudioTransition_CrossFade.h"
#include "Transition.h"
#include "UtilCloneable.h"
#include "UtilLog.h"
#include "UtilLogStl.h"

namespace model { namespace audio {

AudioTransitionFactory::AudioTransitionFactory()
    : TransitionFactory("Audio")
{
    add(boost::make_shared<model::audio::transition::CrossFade>());
}

// static
AudioTransitionFactory& AudioTransitionFactory::get()
{
    static AudioTransitionFactory factory;
    return factory;
}

template < class Archive >
void AudioTransitionFactory::registerTypesForSerialization(Archive& ar)
{
    ar.template register_type<model::audio::transition::CrossFade>();
}

template void AudioTransitionFactory::registerTypesForSerialization<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar);
template void AudioTransitionFactory::registerTypesForSerialization<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar);

}} //namespace