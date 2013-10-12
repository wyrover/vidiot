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

#ifndef CREATE_TRANSITION_HELPER_H
#define CREATE_TRANSITION_HELPER_H

#include "Enums.h"

namespace gui { namespace timeline { namespace command {

/// Create a transition to or from the given clip
/// \param clip the transition must apply to this clip
/// \param type the direction of the transition
/// \param transition the transition effect
void createTransition(model::SequencePtr sequence, model::IClipPtr clip, model::TransitionType type, model::TransitionPtr transition);

}}} // namespace

#endif // CREATE_TRANSITION_HELPER_H