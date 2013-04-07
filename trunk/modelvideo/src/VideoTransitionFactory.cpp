#include "VideoTransitionFactory.h"

#include "Transition.h"
#include "UtilCloneable.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "VideoTransition.h"
#include "VideoTransition_CrossFade.h"

namespace model { namespace video {

VideoTransitionFactory::VideoTransitionFactory()
    : TransitionFactory("Video")
{
    add(boost::make_shared<model::video::transition::CrossFade>());
}

template < class Archive >
void VideoTransitionFactory::registerTypesForSerialization(Archive& ar)
{
    ar.template register_type<model::video::transition::CrossFade>();
}

template void VideoTransitionFactory::registerTypesForSerialization<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar);
template void VideoTransitionFactory::registerTypesForSerialization<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar);

}} //namespace