#ifndef MODEL_VIDEO_TRANSITION_FACTORY_H
#define MODEL_VIDEO_TRANSITION_FACTORY_H

#include "TransitionFactory.h"

namespace model { namespace video {

class VideoTransitionFactory : public TransitionFactory
{
public:
    VideoTransitionFactory();

    static VideoTransitionFactory& get();

    template < class Archive >
    void registerTypesForSerialization(Archive& ar);
};

}} // namespace

#endif // MODEL_VIDEO_TRANSITION_FACTORY_H