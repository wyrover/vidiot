// Copyright 2016 Eric Raijmakers.
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

#pragma once

namespace model { namespace video { namespace transition {

/// Given two sides of a triangle, compute the hypothenusa
/// \param x side 1
/// \param y side 2
/// \return length of hypothenusa
double pythagoras(double x, double y);

/// Determine the distance between two points
int euclidianDistance(int x_origin, int y_origin, int x, int y);

/// Determine the angular distance of a point
/// This is the difference in angle between the lines
/// (x_origin, y_origin) -> (x_origin,0), AND
/// (x_origin, y_origin) -> (x,y)
/// Thus, the clockwise angular distance from 12:00.
int angularDistance(int x_origin, int y_origin, int x, int y);

/// Compute the factor, given a bandsize and a length.
/// \param bandsize length of the total band
/// \param distance part of the band that is covered currently for a given point
/// \param factor transition progress (0 < factor < 1)
/// \param reversed_animation if true, reverse the result
/// \return opacity factor to be applied
float getFactor(int bandsize, int distance, double factor, bool reversed_animation, bool smooth);

}}} // namespace
