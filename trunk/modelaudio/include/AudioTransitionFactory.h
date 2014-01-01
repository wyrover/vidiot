// Copyright 2013,2014 Eric Raijmakers.
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
};

}} // namespace

#endif // MODEL_AUDIO_TRANSITION_FACTORY_H