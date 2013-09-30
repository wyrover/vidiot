// Copyright 2013 Eric Raijmakers.
//
// This file is part of Vidiot.
//
// Vidiot is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Vidiot is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Vidiot. If not, see <http://www.gnu.org/licenses/>.

#include "VideoTransitionFactory.h"

#include "Transition.h"
#include "UtilCloneable.h"
#include "UtilLog.h"
#include "UtilLogStl.h"
#include "VideoTransition.h"
#include "VideoTransition_CrossFade.h"
#include "VideoTransition_Bands.h"

namespace model { namespace video {

VideoTransitionFactory::VideoTransitionFactory()
    : TransitionFactory("Video")
{
    setDefault(boost::make_shared<model::video::transition::CrossFade>());
    add(std::make_pair("Cross fade", "Fade"), boost::make_shared<model::video::transition::CrossFade>());
    add(std::make_pair("Cross fade", "Bands"), boost::make_shared<model::video::transition::Bands>());
}

template < class Archive >
void VideoTransitionFactory::registerTypesForSerialization(Archive& ar)
{
    ar.template register_type<model::video::transition::CrossFade>();
    ar.template register_type<model::video::transition::Bands>();
}

template void VideoTransitionFactory::registerTypesForSerialization<boost::archive::text_oarchive>(boost::archive::text_oarchive& ar);
template void VideoTransitionFactory::registerTypesForSerialization<boost::archive::text_iarchive>(boost::archive::text_iarchive& ar);

}} //namespace