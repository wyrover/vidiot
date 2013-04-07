#ifndef MODEL_AUDIO_TRANSITION_FACTORY_H
#define MODEL_AUDIO_TRANSITION_FACTORY_H

#include "TransitionFactory.h"
#include "UtilSingleInstance.h"

namespace model { namespace audio {

class AudioTransitionFactory
    : public TransitionFactory
    , public SingleInstance<AudioTransitionFactory>
{
public:

    AudioTransitionFactory();

    template < class Archive >
    void registerTypesForSerialization(Archive& ar);
};

}} // namespace

#endif // MODEL_AUDIO_TRANSITION_FACTORY_H