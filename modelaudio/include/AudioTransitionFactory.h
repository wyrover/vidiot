#ifndef MODEL_AUDIO_TRANSITION_FACTORY_H
#define MODEL_AUDIO_TRANSITION_FACTORY_H

#include "TransitionFactory.h"

namespace model { namespace audio {

class AudioTransitionFactory : public TransitionFactory
{
public:
    AudioTransitionFactory();

    static AudioTransitionFactory& get();

    template < class Archive >
    void registerTypesForSerialization(Archive& ar);
};

}} // namespace

#endif // MODEL_AUDIO_TRANSITION_FACTORY_H