// Copyright 2013-2016 Eric Raijmakers.
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

#include "VideoTransition_CrossFade.h"
#include "VideoTransition_FadeToColor.h"
#include "VideoTransition_ImageGradient.h"
#include "VideoTransition_ImageZoom.h"
#include "VideoTransition_WipeArc.h"
#include "VideoTransition_WipeCircle.h"
#include "VideoTransition_WipeDoubleClock.h"
#include "VideoTransition_WipeClock.h"
#include "VideoTransition_WipeStraight.h"

namespace model { namespace video {

VideoTransitionFactory::VideoTransitionFactory()
    : TransitionFactory("Video")
{
    add(boost::make_shared<model::video::transition::CrossFade>());
    add(boost::make_shared<model::video::transition::FadeToColor>());
 // todo   add(boost::make_shared<model::video::transition::ImageGradient>());
    add(boost::make_shared<model::video::transition::ImageZoom>());
    add(boost::make_shared<model::video::transition::WipeArc>());
    add(boost::make_shared<model::video::transition::WipeCircle>());
    add(boost::make_shared<model::video::transition::WipeDoubleClock>());
    add(boost::make_shared<model::video::transition::WipeClock>());
    add(boost::make_shared<model::video::transition::WipeStraight>());
}

}} //namespace