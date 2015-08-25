// Copyright 2013-2015 Eric Raijmakers.
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

#ifndef HELPER_DETAILS_H
#define HELPER_DETAILS_H

#include "Enums.h"

namespace gui { namespace timeline {
    class DetailsClip;
}}

namespace test {

gui::timeline::DetailsClip* DetailsClipView();

void ASSERT_NO_DETAILSCLIP();
void ASSERT_DETAILSCLIP(model::IClipPtr clip);

/// Check the properties of the current details view and the given clip
/// \pre The given clip must be selected and shown in the details view
void ASSERT_CLIPPROPERTIES(
    model::IClipPtr clip,
    model::VideoScaling scaling,
    boost::rational<int> scalingfactor,
    model::VideoAlignment alignment,
    wxPoint position,
    boost::rational<int> rotation);

} // namespace

#endif